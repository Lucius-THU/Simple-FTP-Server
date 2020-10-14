from ftplib import FTP

port = 4021
ftp = FTP()
print(ftp.connect("localhost", port))
print(ftp.login())
print(ftp.sendcmd("CWD public"))
print(ftp.sendcmd("PWD"))
ftp.set_pasv(False)
with open("a.txt","wb") as  f:
      print(ftp.retrbinary('LIST', f.write, 1024))
ftp.close()
