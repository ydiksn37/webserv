import os
import http.cookies
import sys
import time

http_cookie = os.environ.get("HTTP_COOKIE")
print(http_cookie,file=sys.stderr);
cookie = http.cookies.SimpleCookie()
cookie.load(http_cookie)
time.sleep(20);

print("Content-Type: text/html\r\n",end="")
print("Set-Cookie: foo=bar\r\n\r\n",end="")
print("<html><body>")
print("<h1>Hello from Python CGI!</h1>")
print("<p>REQUEST_METHOD: {}</p>".format(os.environ.get("REQUEST_METHOD", "N/A")))
print("<p>QUERY_STRING: {}</p>".format(os.environ.get("QUERY_STRING", "N/A")))
print("<p>COOKIE:","foo = {}</p>".format(cookie["foo"].value) if "foo" in cookie else "foo is not exist")
print("</body></html>")
