from ftplib import FTP

port = 4021
ftp = FTP()
print(ftp.connect("localhost", port))
print(ftp.login())
print(ftp.sendcmd("CWD public"))
print(ftp.sendcmd("PWD"))
ftp.set_pasv(True)
with open("list","wb") as  f:
      print(ftp.retrbinary('LIST', f.write))
filename = "36282579.jpg"
print(ftp.storbinary('STOR %s' % filename, open(filename, 'rb')))
ftp.close()
