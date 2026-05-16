import os

print("Content-Type: text/html\r\n\r\n",end="")
print("<html><body>")
print("<h1>Hello from Python CGI!</h1>")
print("<p>REQUEST_METHOD: {}</p>".format(os.environ.get("REQUEST_METHOD", "N/A")))
print("<p>QUERY_STRING: {}</p>".format(os.environ.get("QUERY_STRING", "N/A")))
print("</body></html>")
