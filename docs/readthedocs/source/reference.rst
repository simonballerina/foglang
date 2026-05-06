.. _theref:

Reference
=========

Content
-------

  * :ref:`1. Introduction<r1>`
  * :ref:`2. Good practice<r2>`

    * :ref:`2.1 Naming conventions<r2_1>`
    * :ref:`2.2 Indents<r2_2>`
    * :ref:`2.3 Comments<r2_3>`
  * :ref:`3. Comments<r3>`
  * :ref:`4. Keywords<r4>`

    * :ref:`4.1 Band<r4_1>`

      * :ref:`4.1.1 Slip <r4_1_1>`
      * :ref:`4.1.2 Grip<r4_1_2>`
    * :ref:`4.2 Foug<r4_2>`

      * :ref:`4.2.1 Svets <r4_2_1>`
      * :ref:`4.2.2 Junk <r4_2_2>`
    * :ref:`4.3 Givet<r4_3>`
    * :ref:`4.4 Naer<r4_4>`
    * :ref:`4.5 Tpos<r4_5>`

      * :ref:`4.5.1 Svets <r4_5_1>`
    * :ref:`4.6 Main<r4_6>`
    * :ref:`4.7 Boul<r4_7>`

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
        givet att i % 3 = 0 {
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

Band is the Foglang keyword for declaring and modifying variables. The syntax is the keyword itself, 
then the variable where the result of the content to the right of the equals sign is stored.
::
    band foo = 120;

Entire mathematical expressions can be evaluated in band, 
but beware that mixing different data types in a single evaluation will result in an error.
::
    band bar = ((120^4)+10)/2;

Strings in Foglang start and end with quotation marks...
::
    band example_str = "This is a string!";

... and can also be concatenated with the '+' operator.
::
    band sentence = "Hello, " + "World!";

The third and last data type in Foglang is the list. A list is declared with brackets, and commas separating the items. 
They act as a single variable storing multiple variables. 
::
    band fruit_list = ["Apple", "Banana", "John", 26];

Note that a list can hold different data types, including other lists.
::
    band names = ["John", "Sven", ["Apple", 26]];

Lists can be changed and accessed with indexing, where the first item holds the index 0.
::
    # Change the first item of a list
    band names = ["John", "Sven", ["Apple", 26]];
    band names[0] = "Enrique";

    # Access the first name
    band first = names[0];

    # Access the string "Apple"
    band item_apple = names[2][0];

If a list is indexed with a negative number, it counts backwards.
:: 
    band names = ["John", "Sven", "Apple"];

    band last = names[-1]; # Apple

Keep in mind that band is needed even for updating variables.


.. _r4_1_1:

Slip
####

The slip tag is used for reading files. 
The evaluated expression to the right of the equals sign is the name of the file being read. 
::
    band slip passwords = "passwords.txt";

Note that the text in passwords.txt will be saved as a string.

.. _r4_1_2:

Grip
####

The grip tag is used for reading input.
The evaluated expression to the right of the equals sign is the promt of the input. 
Can also be left as an empty string.
::
    band grip fruit = "Name a fruit beginning with the letter 'B': ";


.. _r4_2:

Foug
****

The foug keyword is the *only way* to display output in Foglang. It prints directly to standard out (stdout).
::
    foug "Hello, World!\n";

Foug can also directly print variables.
::
    band number = 180;
    foug number;

A newline character is included if a variable is printed. 

.. _r4_2_1:

Svets
#####

The svets tag is used for when you want to merge a string with a variable. 
To print the variable, encase it in percent symbols and insert it into a string.
::
    band age = 30;
    foug svets "I am %age% years old!\n";

Svets will also work for the tpos keyword.

.. _r4_2_2:

Junk
####

The junk tag directs output to standard error, stderr. It can be used for throwing errors in Foglang.
::
    foug junk "ERROR: Something went wrong.\n";

Different tags can also be combined. If both junk and svets are used, 
keep in mind that it's very *foggy* to put the junk before svets.
::
    band err_msg = "Unknown error";
    foug junk svets "ERROR: %err_msg%\n";


.. _r4_3:

Givet
*****

The givet keyword starts a block that runs if a condition is true. 
The keyword is immediately followed by ``att``, and then the condition.
::
    band num = 7;
    givet att num = 7 {
        foug svets "num is equal to 7!\n";
    } 

This code block will only run when num is equal to 7. 
The other comparative operators are greater than ``>``, less than ``<``, and not equal to ``!=``. 


Multiple conditions can also be checked with ``och`` (and), ``eller`` (or) and ``inte`` (not).
::
    band num = 4;
    
    givet att num > 0 och inte num2 > 10 {
        foug "num is between 1 and 10!\n";
    }

.. _r4_4:

Naer
****

The givet keyword starts a block that runs a block while the evaluated condition is true. 
It stops running when the condition is false.
::
    # Count to 10
    band i = 1;
    
    naer i < 11 {
        foug i;
        band i = i+1;
    }

Same as givet, multiple conditions can be checked.
::
    # Count to 10
    band i = 1;
    
    naer i < 10 eller i = 10 {
        foug i;
        band i = i+1;
    }

.. _r4_5:

Tpos
****

Tpos is used for executing code on the host machine. It acts as a terminal directly in Foglang, 
and behaves differently based on your operating system. It can be for example be used to do http requests with curl, 
although using the requests library is recommended.
::
    # Get the contents of example.com
    tpos "curl -s https://example.com -o text.txt";
    
    # Read the output file and print the result
    band slip ret_str = "text.txt";
    foug ret_str;

.. _r4_5_1:

Svets
#####

Tpos can also use the svets tag, which inserts a variable into the call string, just like foug.
::
    # Print the name of the user (on Unix like systems!) using echo
    band cmd = "echo";
    tpos svets "%cmd% $USER";

.. _r4_6:

Main
****

The main keyword is placed at the execution start of a program. Functions are to be placed above it.
::
    # Functions go here...
    main;
    # Code goes here...


.. _r4_7:

Boul
****
The boul keyword defines functions. All functions are to be placed at the top of the program, 
before the main keyword. More on that later. A function is code that can be reused and called from whereever. 
Functions can take arguments separated by commas and must include a return token at the end. 
::
    boul introduce(name, age) {
        foug svets "Hello %name%, you are %age% years old!\n";
        return;
    }

    main;

    introduce("Simon", 26);

A value can also be returned from the function.
::
    boul square(number) {
        return number^2;
    }

    main;
    band squared_num = square(8); # Returns 64