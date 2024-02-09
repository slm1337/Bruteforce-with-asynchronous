# Pipe Hack App

This is a client/server application illustrating an example of bruteforce hacking. Communication between client and server is implemented using pipe channels. The client can asynchronously bruteforce up to 4 logins at once. The server is obviously also capable of asynchronously working with 4 logins simultaneously.


## How to use

Before launching applications, you need to additionally create a text file with the logins and passwords for the hack. An example of such a text file:

    5 5
    login1 testpassword1337
    login2 zxc
    login3 VoVa'
    login4 ShTa
    login5 avov0

- Line 1 of the file is custom, it has no meaning for now.
- Line 2 of the file is a test "login password" to test the connection.
- 3-5 lines of the file - logins and passwords to be cracked by the client application.
  
After successfully creating the required text file, proceed to launch both applications (client.exe & server.exe).

![](/readme-imgs/1.jpg)

On startup, the server application will ask you to select a text file with logins and password. After selecting the file, the server goes into the mode of waiting for data from the client. At startup in the client a choice is available between:
- communication check
- cracking mode

Communication test involves the user entering a test login and password with a ":" sign. The cracking mode provides for the user to enter the maximum possible password length, as well as a choice between three alphabets:
- small Latin letters and apostrophe - 27 characters, 
- small and capital Latin letters and apostrophe + digits - 63 characters;
- small and large Latin letters and apostrophe + digits + small and large Cyrillic letters - 128 symbols;

After selecting one of the three alphabets, an asynchronous cracking process is started.

![](/readme-imgs/2.jpg)

___
### Tested on Windows 7+
____
### Minimum set of tools on which the compilation was tested:
#### - Visual Studio 2019/2022
#### - MSVC v142/143
#### - C++ 14