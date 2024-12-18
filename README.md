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
<ul>
 <li>goto : go to directory</li>
 <li>di : directory items (files and directories)</li>
 <li>back : go back to the last working directory</li>
 <li>cf : create file</li>
 <li>rf : remove file</li>
 <li>yap : echo to stdout or file</li>
 <li>look : see file contents</li>
 <li>exit : exit out of the shell</li>
</ul>
Example of usage:
<p>
 >>> goto /home/user/Documents
 >>> cf file1.txt file2.txt
 >>> yap Hello, world! #=> file1.txt
 >>> look file1.txt
 >>> rf file2.txt
 >>> back
 >>> exit
</p>
