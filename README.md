<h2>Dependencies:</h2>
out123 and mpg123</br>

To install on mac with brew:</br>

```bash
$ brew install out123
$ brew install mpg123
```

To install on linux:

```bash
$ sudo apt install libout123-dev
$ sudo apt install libmpg123-dev
```

<h2>Build</h2>
Build the project using the provided Makefile by using the command 'make'.
If you get an error saying 'out123.h' or 'mpg123.h' could not be found, you will have to add the path to the libraries in the Makefile to GXXFLAGS, but this should not be a problem if the libraries installed in the correct location.

https://www.rapidtables.com/code/linux/gcc/gcc-l.html
https://www.rapidtables.com/code/linux/gcc/gcc-i.html

<h2>Run</h2>
Run the project with ./main

Within the program, use the command 'help' to see information about how the use the program.
