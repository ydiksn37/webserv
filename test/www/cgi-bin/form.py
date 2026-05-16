import os
import http.cookies
import sys

http_cookie = os.environ.get("HTTP_COOKIE")
print(http_cookie,file=sys.stderr);
cookie = http.cookies.SimpleCookie()
cookie.load(http_cookie)

print("Content-Type: text/html\r\n",end="")
print("Set-Cookie: foo=bar\r\n\r\n",end="")
print("<html><body>")
print("<h1>Post Form</h1>")
print("<form action=\"/cgi-bin/form.py\" method=\"POST\">text:<input type=\"text\" name=\"text\"><button type=\"submit\">submit</button></form>")
print(input())
print("</body></html>")
