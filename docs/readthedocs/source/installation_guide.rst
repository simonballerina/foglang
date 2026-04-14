.. _install:

Installation Guide
==================

Requirements
------------
There are a few requirements for your machine, in order to install Foglang. First of, you need the C compiler GCC. Check if you have it by running the following command in your terminal:
::
    gcc -v

Secondly, you need a modern version of python, which you can get `here <https://www.python.org/downloads/>`_. To check if you already have it, run the following command:
::
    python3 -V

Clone repository
----------------

The first step is to download the the Foglang repository, which you can do on our official (`Github page <https://www.github.com/simonballerina/foglang>`_) or by running the following command:
::
    curl https://github.com/simonballerina/foglang/archive/refs/heads/main.zip

If you have `git <https://git-scm.com/install/>`_ installed, you can clone the official Foglang repository using:
::
    git clone https://github.com/simonballerina/foglang

.. note::

    This guide is for Foglang2. If you wish to use Foglang1, compile it on your own after downloading the Foglang repo.

Setting up Foglang command
--------------------------

Once you have the Foglang repo, install the Foglang command using the following command, from the Foglang root:
::
    python3 docs/foglang2/install.py

The current installer will output in Swedish. Use a store-bought Swedish-English dictionary, or other methods, to understand and follow the installation.

And you are all set!
--------------------
Once the command is installed you are free to run Foglang to your hearts content! To run a Foglang file use the following:
::
    foglang2 [FILE]

To learn more about our beautiful language, check out the :ref:`reference<theref>`.

Fog Fog Fog Fog 

.. figure:: ../img/foggy.avif
    :alt: Foggy landscape
    :align: center

    Photo by `Nathan Anderson <https://unsplash.com/photos/fogy-sky-v1pu3WSFieE>`_ on `Unsplash <https://unsplash.com>`_