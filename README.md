#customshell

Custom Shell is a purpose built restricted command shell to run on a Linux System only allowing access to a limited set of functionality for a specific User.
 
Built in allowed commands: cd, help, exit, pw (pwd), dt (date time set to specific format), ifc (ifconfig eth0) and ud (userID, groupID, username, groupname, iNode of user’s home directory)

#Setup
- Download and compile the customshell.c to the /usr/bin directory of an System Administrator
- Create a new user, setting the customshell as the default shell (chsh -s /usr/bin/customshell NewUser)
- Login as the new user and the customshell should be displayed like so:

======= CUSTOM SHELL INITITATED =======
 
Type 'help' to get a list of allowed commands
 
======= CUSTOM SHELL INITITATED =======
