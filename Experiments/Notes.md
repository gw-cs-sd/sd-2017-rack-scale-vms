Rack Scale VMs Notes on Experimentation
==

Experiments:
--

  - Possible applications:
    + Memcached, MongoDB, Redis, DNA and Genome Sequencing
    + **Goal:** Show that a memory dependent application incurs clear performance gains when the RSA mem-pool size is increased
      - In short, we are fooling a process to think it can use more memory than what is available.
    + Application justification:
      - Need an application that clearly incurs a performance hit when it has less memory VS more memory
      - Need an application where you can throttle its memory utilization
      - Need an application that has a skewed, yet controllable, memory footprint (density of hot VS cold pages)
    + Output and target analysis:
      - Graph (line or bar)
        + X-Axis: Mem-pool size
        + Y-Axis: Performance
        + Lines/Runs:
          - All pages local
          - Uniformly hot (1/2 pages local, 1/2 pages remote)
          - Skewed page access
      - Goal: (all local) > (skew) > (uniform)

Benchmarking Tools:
--

  - Wrk - HTTP Benchmarking tool: [https://github.com/wg/wrk](https://github.com/wg/wrk)
  - YCSB - Cloud System Benchmarking Suite [https://github.com/brianfrankcooper/YCSB](https://github.com/brianfrankcooper/YCSB)
