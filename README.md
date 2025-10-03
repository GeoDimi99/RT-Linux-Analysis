# Real Time Linux

Linux, originally, is designed to be a **time-sharing system**, i.e. the gool is give the best throughput from the hardware using all the resources at maximum, but it's also possible to make **real-time system**, i.e. the goal is the determinism even at a low global throughput. 

There are two different approaches to make Linux real-time system, in my case I use the **Single Kernel Approach** (with PREEMPT_RT) that consist to modify the Linux Kernel itself in order to get required latencies.

## Setup System

Following installation instructions are tested on VM (Virtual Box) running 64bit Ubuntu Server 24.04 LTS with 6.8.0-generic kernel.

### Kernel and Patch Download

- We need some tools to build the kernel
  
  ```bash
  sudo apt update 
  sudo apt install  build-essential libssl-dev libelf-dev \
                    libncurses5-dev  flex bison bc 
  ```

- Make a directory named **kernel** in the desired location:
  
  ```bash
  mkdir -p ~/kernel
  cd ~/kernel
  ```

- Print kernel version and machine related information, here we have linux kernel version 6.8.0, I would prefere to build and patch nearest kernel version to existing one.
  
  ```bash
  uname -a
  ```

- Download the Kernel and the Patch PREEMPT_RT
  
  ```bash
  wget https://mirrors.edge.kernel.org/pub/linux/kernel/v6.x/linux-6.8.tar.xz
  wget https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt/6.8/older/patch-6.8-rt8.patch.xz
  ```

### Configure the settings for PREEMPT_RT

- Decompress and apply the patch
  
  ```bash
  xz -cd linux-6.8.tar.xz | tar xvf -
  cd linux-6.8/
  xzcat ../patch-6.8-rt8.patch.xz | patch -p1 --verbose
  ```

- To ensure that the RT kernel supports the current distribution, we need to copy current configuration
  
  ```bash
  cp /boot/config-$(uname -r) .config
  ```

- Keep default settings by automatically setting yes to old configuration.
  
  ```bash
  yes '' | make oldconfig
  ```

- Menuconfig allows us to choose linux features, in this case PREEMPT_RT patch related functionality
  
  ```bash
  make menuconfig
  ```

- Timer tick handling (Full dynticks system (tickless))
  
  ```
  # Enable CONFIG_NO_HZ_FULL
   -> General setup
    -> Timers subsystem
     -> Timer tick handling (Full dynticks system (tickless))
      (X) Full dynticks system (tickless)
  ```

- Preemption Model (Fully Preemptible Kernel (Real-Time))
  
  ```
  # Enable CONFIG_PREEMPT_RT
   -> General Setup
    -> Preemption Model (Fully Preemptible Kernel (Real-Time))
     (X) Fully Preemptible Kernel (Real-Time)
  ```

- Timer frequency
  
  ```
  # Set CONFIG_HZ_1000 (note: this is no longer in the General Setup menu, go back twice)
   -> Processor type and features
    -> Timer frequency (1000 HZ)
     (X) 1000 HZ
  ```

- Default CPUFreq governor
  
  ```
  # Set CONFIG_HZ_1000 (note: this is no longer in the General Setup menu, go back twice)
   -> Processor type and features
    -> Timer frequency (1000 HZ)
     (X) 1000 HZ
  ```

- In your kernel configuration file specifically when compiling on Ubuntu (debian)to avoid any error during compilation process related to SYSTEM_TRUSTED_KEYS and CONFIG_SYSTEM_REVOCATION_KEYS update following lines
  
  ```bash
  scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
  scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
  ```

### Compile and Install Kernel

- Compile the kernel (It could take time, in my case 9 h)
  
  ```bash
  sudo make
  ```

- Install the kernel, update grub and reboot
  
  ```bash
  sudo make modules_install
  sudo make install 
  sudo update-grub
  reboot
  ```

- Check the kernel version (it could appear PREEMPT_RT )
  
  ```bash
  uname -a
  ```

## Experiments and Analysis

In the follow for perform the experiments we need to install some tools before (tmux, git and docker):

- install the tools:
  
  ```bash
  sudo apt update
  sudo apt install tmux git docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
  ```

- clone the repository:
  
  ```bash
  git clone https://github.com/GeoDimi99/RT-Linux-Analysis.git
  cd RT-Linux-Analysis
  ```

### RT Basic Tests

The follow experiments are inspired from [Understanding Linux Scheduling](https://www.linkedin.com/pulse/20140629145049-21586023-understanding-linux-scheduling/). The experiment consist of three different codes that use the three different scheduler (i.e. SCHED_OTHER, SCHED_FIFO and SCHED_RR). 

The scenarios analyzed are:

- **Two FIFO tasks (same priority)**

- **Two RR tasks (same priority)**

- **Two RR tasks (different priorities)**

- **Two RR tasks and one FIFO task (same priority)**

- **One OTHER task and one FIFO task**



#### Exp 1: Two FIFO task (Same Priority)

In the first experiment, the system under consideration is single-core. Two tasks were created, each executing an infinite loop, and both were scheduled under **SCHED_FIFO** with identical priority (priority level = 1).

The first task (green) was launched and immediately began producing output. When the second task (red) was started, it remained blocked and did not generate any output until a `CTRL+C` signal was issued.

The experimental results are as follows:

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-17-20-image.png)

After `CTRL+C`:

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-17-58-image.png)

The corresponding execution pattern can be modeled as follows:


![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-22-15-23-image.png)



#### Exp 2: Two RR task (Same Priority)

The second experiment involved two tasks scheduled under **SCHED_RR** with identical priority. In this case, both tasks successfully alternated in producing output.

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-23-50-image.png)

This behavior is consistent with the **Round Robin** policy, whereby tasks of the same priority share the CPU in time slices. The duration of each time slice is determined by the system parameter:`/proc/sys/kernel/sched_rr_timeslice_ms`.

The expected execution pattern is depicted below:



![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-22-18-image.png)





#### Exp 3: Two RR task (different priority)

The third experiment analyzed the effect of different priorities under **SCHED_RR**. Two tasks were launched: the first with priority 1 and the second with priority 2. The lower-priority task was started first, but upon initiation of the higher-priority task, the former was immediately preempted.

The higher-priority task continued to execute exclusively until a `CTRL+C` signal was sent, after which the lower-priority task resumed.

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-35-23-image.png)

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-35-48-image.png)

The execution can be represented as:

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-37-15-image.png)



#### Exp 4: Two RR task and one FIFO task (Same Priority)

The fourth experiment considered the interaction between **SCHED_RR** and **SCHED_FIFO**. Two RR tasks were launched together with one FIFO task, all configured with the same priority.

Initially, the two RR tasks alternated execution and produced output concurrently. Once the FIFO task was scheduled, however, it monopolized the CPU, preventing further execution of the RR tasks.

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-46-42-image.png)

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-47-21-image.png)



This illustrates the higher scheduling precedence of FIFO over RR when priorities are equal. The corresponding execution model is:

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-49-05-image.png)





#### Exp 5: One OTHER task and one FIFO task

The final experiment investigated the interplay between a **SCHED_OTHER** task (non-real-time) and a **SCHED_FIFO** task (real-time).  
In principle, real-time tasks should always preempt non-real-time tasks. Consequently, one would expect the **OTHER** task to be suspended once the FIFO task begins execution.

However, the observed results diverged from this expectation:The empirical behavior suggests periodic resumption of the non-real-time task, as illustrated below:

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-53-22-image.png)

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-53-42-image.png)

The empirical behavior suggests periodic resumption of the non-real-time task, as illustrated below:

![](C:\Users\dimit\AppData\Roaming\marktext\images\2025-09-24-23-57-13-image.png)

This anomaly can be explained by two kernel parameters governing the allocation of CPU time to real-time tasks, both accessible via the `/proc` filesystem:

- `/proc/sys/kernel/sched_rt_period_us`  
  (default value = `1000000` µs, i.e., 1 second)

- `/proc/sys/kernel/sched_rt_runtime_us`  
  (default value = `950000` µs, i.e., 0.95 seconds), which defines the maximum fraction of CPU time that real-time tasks may consume.

These constraints prevent real-time tasks from monopolizing the CPU indefinitely, thereby ensuring a minimal execution window for non-real-time tasks.
