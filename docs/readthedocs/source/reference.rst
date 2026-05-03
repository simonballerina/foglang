.. _theref:

Reference
=========

Content
-------

  * :ref:`1. Introduction<r1>`
  * :ref:`2. Good practice<r2>`

    * :ref:`2.1 Naming conventions<r2_1>`
    * :ref:`2.1 Indents<r2_2>`
    * :ref:`2.1 Comments<r2_3>`
  * :ref:`3. Comments<r3>`
  * :ref:`4. Keywords<r4>`


.. _r1:

Introduction
------------

When running Foglang code, the interpreter divides the file into various instructions, separated by semicolons.
Therefore, each new instruction needs to be terminated by a semicolon.

.. _r2:

Good practice
-------------

aka. best practices — We <3 god praxis

.. _r2_1:

Naming conventions
******************

How to name variables, functions etc. in your code is completely up to you as a coder, but it is recommended to implement a personal standard.
It can also be beneficial to use general standards for the Foglang language. The general standard is to use snake_case for all names in Foglang. Below is an example:
::
    band student_name = "Simon";
    band student_birth_year = 2008;

.. _r2_2:

Indents
*******

It is good practice to indent your code for each block.
Tabs and spaces are all ignored by the interpreter, so the amount or type is up to personal preference.
Here is a program using quadruple spaces intents:
::
    band i = 0;
    # naer start a code block
    naer i < 100 {
        # indent
        # givet att also starts a block
        givet att i % 3 == 0 {
            foug svets "%i% is divisible by three\n";
        }
        # unindent after block end
    }

.. _r2_3:

Comments
********

It is good practice to use comments in your code to explain what it is doing.
The amount of comments recommended depends heavily on use-case. Learn about using comments :ref:`here<r3>`.

.. _r2_4:

Other best practices
********************

To learn more about good practice, read the `wikipedia entry <https://en.wikipedia.org/wiki/Coding_best_practices#Keep_the_code_simple>`_ for coding best practices.

.. _r3:

Comments
--------

Foglang has two different ways of making comments: single line, and block comments.
Single line comments begin after a hashtag (`#`) and runs until the end of the line.
Block comments begin with a hashtag followed by a star (`#*`) and continues until the reverse appears (`*#`).
::
    # this is a single line comment — the following line is not a comment
    band foo = 8;
    #* <- this starts the comment
    This is a multiline comment
    It is still a comment here
    this ends the comment -> *#

.. _r4:

Keywords
--------

.. _r4_1:

Band
****

etc

.. _r4_2:

Slip
####

tbd — be till gud och vipps...