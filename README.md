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
  cd -p ~/kernel
  ```

- Print kernel version and machine related information, here we have linux kernel version 6.8.0, I would prefere to build and patch nearest kernel version to existing one.
  
  ```bash
  uname -a
  ```

- Download the Kernel and the Patch PREEMPT_RT
  
  ```bash
  wget https://mirrors.edge.kernel.org/pub/linux/kernel/v6.x/linux-6.8.tar.xz
  wget https://mirrors.edge.kernel.org/pub/linux/kernel/projects/rt/6.8/older/patch-6.8rt8.patch.xz
  ```
  
  

### Configure the settings for PREEMPT_RT

- Decompress and apply the patch
  
  ```bash
  xz -cd linux-6.8.tar.xz | tar xvf -
  cd linux-6.8/
  xzcat ../patch-6.8-rt8.patch.xz | patch -p1 –verbose
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

So I analyse the follow cases:

- **two fifo task (same priority)**:

- **two rr task (same priority)**:

- **two rr task (different priority)**:

- **two rr task and one fifo task (same priority)**:

- **one other task and one fifo task**:


