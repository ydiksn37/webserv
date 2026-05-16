#!/usr/bin/env python3
import os
import sys
import http.cookies
import uuid
import json

SESSION_DIR = "/tmp"
os.makedirs(SESSION_DIR, exist_ok=True)

http_cookie = os.environ.get("HTTP_COOKIE", "")
cookie = http.cookies.SimpleCookie(http_cookie)

session_id = None
session_data = {}
is_new_session = False

if "session_id" in cookie:
    session_id = cookie["session_id"].value
    session_file = os.path.join(SESSION_DIR, session_id)
    
    if os.path.exists(session_file):
        try:
            with open(session_file, "r") as f:
                session_data = json.load(f)
        except json.JSONDecodeError:
            pass

if not session_data:
    session_id = uuid.uuid4().hex
    session_data = {"visits": 0}
    is_new_session = True

session_data["visits"] += 1

session_file = os.path.join(SESSION_DIR, session_id)
with open(session_file, "w") as f:
    json.dump(session_data, f)


print("Content-Type: text/html\r\n", end="")

if is_new_session:
    print(f"Set-Cookie: session_id={session_id}; HttpOnly; Path=/\r\n", end="")

print("\r\n", end="")

print("<html><body>")
print("<h1>Python CGI Session Demo</h1>")

if is_new_session:
    print("<p style='color: green;'>Start new session!</p>")
else:
    print("<p style='color: blue;'>Keep current session!</p>")

print("<h2>STATUS</h2>")
print(f"<p>Session ID: <strong>{session_id}</strong></p>")
print(f"<p>visit count: <strong>{session_data['visits']}</strong></p>")

print(f"<p>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'N/A')}</p>")
print(f"<li><a href=\"/\">Back</a></li>");
print("</body></html>")
