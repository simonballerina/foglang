# Foglang
Överlag ganska fogligt.

## Documentation
Read our documentation at our [readthedocs page](https://foglang.readthedocs.io)

## Installation Guide

> **_NOTE:_**  The following istallation guide is downstream of our [readthedocs installation guide](https://foglang.readthedocs.io/en/latest/installation_guide.html). Look there to get the latest version.

### Requirements
There are a few requirements for your machine, in order to install Foglang. First of, you need the C compiler GCC. Check if you have it by running the following command in your terminal: `gcc -v`. Secondly, you need a modern version of python, which you can get [here](https://www.python.org/downloads/). To check if you already have it, run the following command: `python3 -V`.

### Clone repository

The first step is to download the the Foglang repository, which you can do on our official [Github page](https://www.github.com/simonballerina/foglang) or by running the following command:
```curl https://github.com/simonballerina/foglang/archive/refs/heads/main.zip```

If you have [git](https://git-scm.com/install/) installed, you can clone the official Foglang repository using:
```git clone https://github.com/simonballerina/foglang```

> **_NOTE:_**  This guide is for Foglang2. If you wish to use Foglang1, compile it on your own after downloading the Foglang repo.

### Setting up Foglang command

Once you have the Foglang repo, install the Foglang command using the following command, from the Foglang root:
```python3 docs/foglang2/install.py```

The current installer will output in Swedish. Use a store-bought Swedish-English dictionary, or other methods, to understand and follow the installation.

### And you are all set!

Once the command is installed you are free to run Foglang to your hearts content! To run a Foglang file use the following:
```foglang2 [FILE]```

Fog Fog Fog Fog 

![image](docs/readthedocs/img/foggy.avif)