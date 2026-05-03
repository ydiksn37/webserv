<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<html><body>";
echo "<h1>Hello from PHP CGI!</h1>";
echo "<p>REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>QUERY_STRING: " . $_SERVER['QUERY_STRING'] . "</p>";
echo "</body></html>";
?>
