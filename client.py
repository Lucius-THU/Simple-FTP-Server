from ftplib import FTP

port = 4021
ftp = FTP()
print(ftp.connect("localhost", port))
print(ftp.login())
print(ftp.sendcmd("CWD public"))
print(ftp.sendcmd("PWD"))
ftp.set_pasv(True)
filename = "1.txt"
print(ftp.storbinary('STOR %s' % filename, open(filename, 'rb')))
print(ftp.storbinary('APPE %s' % filename, open(filename, 'rb')))
ftp.set_pasv(False)
print(ftp.retrbinary('RETR %s' % filename, open(filename, 'wb').write, rest=2))
print(ftp.retrbinary('LIST', open("list", 'wb').write))
ftp.close()
