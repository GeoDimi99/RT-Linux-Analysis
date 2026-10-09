# Real-Time Linux Analysis

Linux was originally designed as a **time-sharing system**: its goal is to maximize overall throughput by keeping all hardware resources as busy as possible. It can also be turned into a **real-time system**, where the goal is **determinism** (predictable, bounded latencies), even at the cost of lower overall throughput.

There are two main approaches to making Linux real-time:

- **Dual-kernel approach**: a small real-time co-kernel runs alongside Linux and handles time-critical tasks (e.g. Xenomai, RTAI).
- **Single-kernel approach**: the Linux kernel itself is modified to achieve the required latencies.

This project uses the **single-kernel approach** with the **PREEMPT_RT** patch.

> **Note:** since Linux 6.12, PREEMPT_RT is part of the mainline kernel, so newer kernels no longer need a separate patch. This guide uses kernel 6.8, which still requires it.

## Table of Contents

- [System Setup](#system-setup)
  - [Prerequisites](#prerequisites)
  - [Download the Kernel and the Patch](#download-the-kernel-and-the-patch)
  - [Configure the Kernel for PREEMPT_RT](#configure-the-kernel-for-preempt_rt)
  - [Compile and Install the Kernel](#compile-and-install-the-kernel)
- [Experiments and Analysis](#experiments-and-analysis)
  - [Setup](#setup)
  - [Basic RT Scheduling Tests](#basic-rt-scheduling-tests)
  - [Summary of Results](#summary-of-results)

---

## System Setup

The following instructions were tested on a **VirtualBox VM** running **Ubuntu Server 24.04 LTS (64-bit)** with the `6.8.0-generic` kernel.

### Prerequisites

Install the tools needed to build the kernel:

```bash
sudo apt update
sudo apt install build-essential libssl-dev libelf-dev \
                 libncurses5-dev flex bison bc
```

### Download the Kernel and the Patch

1. Create a working directory:

   ```bash
   mkdir -p ~/kernel
   cd ~/kernel
   ```

2. Check the current kernel version. It is best to build a kernel version as close as possible to the one already installed (here, 6.8.0):

   ```bash
   uname -a
   ```

3. Download the kernel sources and the matching PREEMPT_RT patch:

   ```bash
   wget https://mirrors.edge.kernel.org/pub/linux/kernel/v6.x/linux-6.8.tar.xz
   wget https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt/6.8/older/patch-6.8-rt8.patch.xz
   ```

### Configure the Kernel for PREEMPT_RT

1. Extract the sources and apply the patch:

   ```bash
   xz -cd linux-6.8.tar.xz | tar xvf -
   cd linux-6.8/
   xzcat ../patch-6.8-rt8.patch.xz | patch -p1 --verbose
   ```

2. Start from the current distribution's configuration, so the new kernel supports the same hardware and features:

   ```bash
   cp /boot/config-$(uname -r) .config
   ```

3. Accept the default value for every new option:

   ```bash
   yes '' | make oldconfig
   ```

4. Open the configuration menu to enable the real-time features:

   ```bash
   make menuconfig
   ```

   Set the following options:

   | Option | Menu path | Value |
   |---|---|---|
   | `CONFIG_NO_HZ_FULL` | General setup → Timers subsystem → Timer tick handling | Full dynticks system (tickless) |
   | `CONFIG_PREEMPT_RT` | General setup → Preemption Model | Fully Preemptible Kernel (Real-Time) |
   | `CONFIG_HZ_1000` | Processor type and features → Timer frequency | 1000 HZ |
   | `CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE` | Power management and ACPI options → CPU Frequency scaling → Default CPUFreq governor | performance |

   Save the configuration and exit.

5. On Ubuntu/Debian, clear the trusted and revocation keys to avoid certificate-related build errors:

   ```bash
   scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
   scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
   ```

### Compile and Install the Kernel

1. Compile the kernel using all available CPU cores:

   ```bash
   make -j$(nproc)
   ```

   > Compilation can take a long time. On a single-core VM it took about **9 hours**; using `-j$(nproc)` on a multi-core machine reduces this considerably. `sudo` is not needed for this step.

2. Install the modules and the kernel, update GRUB and reboot:

   ```bash
   sudo make modules_install
   sudo make install
   sudo update-grub
   sudo reboot
   ```

3. After rebooting, verify that the real-time kernel is running. The output should contain `PREEMPT_RT`:

   ```bash
   uname -a
   ```

---

## Experiments and Analysis

### Setup

1. Install the tools used in the experiments (tmux, git and Docker):

   ```bash
   sudo apt update
   sudo apt install tmux git
   ```

   For Docker, the `docker-ce` packages require Docker's official apt repository to be configured first (see the [Docker install guide for Ubuntu](https://docs.docker.com/engine/install/ubuntu/)). Then:

   ```bash
   sudo apt install docker-ce docker-ce-cli containerd.io \
                    docker-buildx-plugin docker-compose-plugin
   ```

2. Clone this repository:

   ```bash
   git clone https://github.com/GeoDimi99/RT-Linux-Analysis.git
   cd RT-Linux-Analysis
   ```

### Basic RT Scheduling Tests

These experiments are inspired by [Understanding Linux Scheduling](https://www.linkedin.com/pulse/20140629145049-21586023-understanding-linux-scheduling/). They use three programs, one for each Linux scheduling policy:

- **SCHED_OTHER**: the default, non-real-time time-sharing policy.
- **SCHED_FIFO**: real-time, first-in first-out. A task runs until it blocks, yields, or is preempted by a higher-priority task.
- **SCHED_RR**: real-time round-robin. Like FIFO, but tasks with the same priority share the CPU in fixed time slices.

All experiments run on a **single-core** system, and every task executes an infinite loop that prints output.

#### Experiment 1: Two FIFO tasks (same priority)

Two tasks were scheduled under `SCHED_FIFO` with the same priority (1).

The first task (green) started and immediately began producing output. The second task (red), started afterwards, stayed blocked and produced no output until the first task was stopped with `CTRL+C`.

**Why:** a FIFO task never gives up the CPU to a task of equal priority, so the second task waits until the first one terminates.

#### Experiment 2: Two RR tasks (same priority)

Two tasks were scheduled under `SCHED_RR` with the same priority.

Both tasks alternated, producing interleaved output.

**Why:** under round-robin, tasks of equal priority take turns, each running for one time slice before moving to the back of the queue.

#### Experiment 3: Two RR tasks (different priorities)

Two `SCHED_RR` tasks were launched, the first with priority 1 and the second with priority 2. The lower-priority task was started first.

As soon as the higher-priority task started, the lower-priority task was preempted. The higher-priority task ran exclusively until it was stopped with `CTRL+C`, after which the lower-priority task resumed.

**Why:** round-robin time slicing applies only among tasks of the *same* priority; a higher-priority task always preempts a lower-priority one.

#### Experiment 4: Two RR tasks and one FIFO task (same priority)

Two `SCHED_RR` tasks and one `SCHED_FIFO` task were launched, all with the same priority.

At first, the two RR tasks alternated as expected. Once the FIFO task was scheduled, it took over the CPU and the RR tasks no longer ran.

**Why:** FIFO and RR tasks share the same priority queue. When the FIFO task gets the CPU, it has no time slice, so it never yields to the RR tasks.

#### Experiment 5: One OTHER task and one FIFO task

A `SCHED_OTHER` (non-real-time) task and a `SCHED_FIFO` (real-time) task were run together.

In principle, a real-time task should always preempt a non-real-time one, so the OTHER task was expected to stop running once the FIFO task started. **This did not happen:** the OTHER task kept producing some output.

**Why:** Linux limits how much CPU time real-time tasks may consume (*RT throttling*). Two kernel parameters control this:

| Parameter | Default | Meaning |
|---|---|---|
| `/proc/sys/kernel/sched_rt_period_us` | `1000000` µs (1 s) | Length of the accounting period |
| `/proc/sys/kernel/sched_rt_runtime_us` | `950000` µs (0.95 s) | Maximum RT CPU time within each period |

With the defaults, real-time tasks can use at most 95% of each second; the remaining 5% is reserved for non-real-time tasks. This keeps a runaway real-time task from freezing the system.

The limit can be inspected, and disabled for testing by setting the runtime to `-1`:

```bash
cat /proc/sys/kernel/sched_rt_runtime_us
echo -1 | sudo tee /proc/sys/kernel/sched_rt_runtime_us
```

> **Warning:** with throttling disabled, a real-time task in an infinite loop can make a single-core system completely unresponsive.

### Summary of Results

| # | Tasks | Observed behavior |
|---|---|---|
| 1 | 2× FIFO, same priority | The first task runs; the second waits until the first ends |
| 2 | 2× RR, same priority | The tasks alternate |
| 3 | 2× RR, different priorities | The higher-priority task runs exclusively |
| 4 | 2× RR + 1× FIFO, same priority | RR tasks alternate until the FIFO task takes over |
| 5 | 1× OTHER + 1× FIFO | OTHER still runs, because of RT throttling |
