# Custom Shell

Custom Linux Shell written in C.<br />
To run
```
git clone <repo_url>
cd <project_directory>
gcc custom_shell.c -o custom_shell
./custom_shell
```
<br />
List of built-in commands:
 - goto : go to directory
 - di : directory items (files and directories)
 - back : go back to the last working directory
 - cf : create file
 - rf : remove file
 - yap : echo to stdout or file
 - look : see file contents
 - exit : exit out of the shell
<br />
Example of usage:
```
>>> goto /home/user/Documents
>>> cf file1.txt file2.txt
>>> yap Hello, world! #=> file1.txt
>>> look file1.txt
>>> rf file2.txt
>>> back
>>> exit
```
