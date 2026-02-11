# Mavir - Lightweight Antivirus
Lightweight C++ User-Mode antivirus engine inspired by cybersecurity research.

# Reasons behind:
Currently, my main focus is on malware analysis and general cybersecurity threats, with a strong interest in detection and response.<br>
By understanding how malicious software operates, I can design and automate targeted countermeasures. <br>
<br>
The project originally started as a PE file parser and analysis tool, and this functionality will be expanded and reintroduced in future versions.<br>

# Features
- Monitor Startup Folders (User & Admin) T1547.001 – Startup Folder <br>
There are multiple way to achieve Persistence. One of it is to create/move Files into the User and Admin Startup Folder.<br>
Here Actors are  implementing Files (.exe .lnk's) in the Startup Folders. Example Malware: Remcos, Agent Tesla.<br>

- VBs Deactivation & Monitoring <br>
A Thing i had and came across while reading the Book Windows Internals Part 1 was, that vbs Files are correlated to a specific subsystem.<br>
When understanding Subsystems more, i asked myself which reason on the World besides Legacy Systems u would depend on .vbs.<br> Tldr: There are not many reasons out there beside legacy systems<br>
But Malware wants to do this. And not just a few. Out of the Trending Malware on Any.Run these Families are using it: Agent Tesla, Remcos, AsyncRat, Snake Keylogger, XWORM, Quasar RAT.<br>

- Task Scheduler Monitoring <br>
Basically Taskscheduling is a common way for Malware Acteurs to achieve persistence by utilizing it.<br>
This persistence via Taskscheduler is achieved by i think 2 mechanism im not 100% sure. But i know:<br>
It gets saved in a Registry Tree or lets say new Tasks get added there, and there is a xml file which also holds the Tasks scheduled.<br>
By Monitoring those 2 Factors we can almost say certain there is no Taskscheduling without Kernel Tricks. <br>
  - Monitors Keys of HKLM & HKCU in the TaskScheduler Trees by utilizing RegNotifyChangeKeyValue which is a great way to check for changes but keeping performance on a normal level.<br>
  - Checks with ReadDirectoryChangesW Directory changes of the Windows/System32/Tasks Folder also with 0 polling and no perfomance issues :)  <br>


Next: <br>
- Startup Registry Monitoring

Planned:
- Looking into how to integrate WMI <br>
- Looking into ETW Provider Registration & Usage <br>
- Static File Analysis with Heuristik Result <br>
- Extended Handling of Cases<br>
...
  
# Screenshot
<img width="977" height="514" alt="grafik" src="https://github.com/user-attachments/assets/bb717a87-8662-4059-a8f8-fbced2e52acc" />

# Documentation
You can find the Documentation for the Doxygen in the Header Files.<br>
The Doxygen Documentation is in the folder doxygen_docs.<br>
Inside the C++ Files you can expect Code specific comments.<br>

# Usage
1. Clone the Repository
2. Open the Project Solution File and Build it.
3. Create a .lnk file of it and put in the Properties the "Run as Admin" Checkbox.
4. Now move that .lnk in a Startup Folder (WIN-KEY + R, shell:startup.)
