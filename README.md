*This project has been created as part of the 42 curriculum by yukusano, sonakamu, ssawa.*

# Webserv

## Description

Webserv is a small HTTP/1.1 server written in C++98. It implements a subset of
the behavior expected from a production web server while keeping the codebase
focused on the core concepts of network programming, request parsing, routing,
and response generation.

The server can:

- Serve static files from configurable document roots.
- Listen on multiple ports.
- Select a server block by port and `Host` header.
- Match requests to the longest compatible `location` prefix.
- Handle `GET`, `POST`, and `DELETE`.
- Generate default and custom error pages.
- Enforce per-location allowed methods.
- Enforce configurable client body size limits.
- Decode chunked request bodies.
- Generate directory listings when `autoindex` is enabled.
- Store uploaded request bodies in configured upload directories.
- Execute CGI scripts based on file extension.

The implementation is organized into separate modules:

- `srcs/config`: configuration tokenizer, parser, validation, server/location lookup.
- `srcs/Http`: HTTP request parsing and response serialization.
- `srcs/engine`: route resolution, method dispatch, static files, upload, delete, CGI selection.
- `srcs/eventloop`: sockets, epoll event loop, client state, CGI pipe integration.

## Instructions

Build the server from the repository root:

```sh
make
```

Run with an explicit configuration file:

```sh
./webserv configurations/test.conf
```

Run without an argument to use the default configuration:

```sh
./webserv
```

The default path is:

```text
configurations/default.conf
```

Run the unit tests:

```sh
make test
```

Available example configurations:

- `configurations/default.conf`: minimal static server on port `8080`.
- `configurations/test.conf`: static files, CGI, upload, delete, chunked request body tests.
- `configurations/cgi.conf`: CGI-focused server on port `8082`.
- `configurations/example.conf`: multiple server blocks, multiple ports, aliases, redirects, upload, CGI.

Example requests:

```sh
curl -v http://localhost:8080/
curl -v http://localhost:8080/files/existing.txt
curl -v -X POST --data-binary @test/upload/webserv_upload_payload.txt \
  http://localhost:8080/upload/webserv_upload_payload.txt
curl -v -X DELETE http://localhost:8080/upload/webserv_upload_payload.txt
curl -v "http://localhost:8080/cgi-bin/hello.py?name=webserv"
```

Chunked upload example:

```sh
printf 'POST /upload/chunked.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\nb\r\nhello world\r\n0\r\n\r\n' | nc localhost 8080
```

Stop the server with `Ctrl-C`.

## Configuration

Configuration files use an nginx-inspired syntax with `server` and `location`
blocks. Supported directives include:

- `listen`
- `server_name`
- `root`
- `index`
- `client_max_body_size`
- `error_page`
- `allow_methods` or `allow_method`
- `autoindex`
- `return`
- `alias`
- `upload_enable`
- `upload_store`
- `cgi_extension`
- `cgi_path`

Locations inherit selected server-level settings such as `root`, `index`,
`client_max_body_size`, and `error_page`, unless overridden inside the location.

## Notes

This project is intended to run in the 42 Linux evaluation environment. The
event loop uses Linux `epoll`.

CGI examples are configured for common interpreter paths:

- Python: `/usr/bin/python3`
- PHP CGI: `/usr/bin/php-cgi`

If an interpreter is not installed at the configured path, the corresponding CGI
request will fail until the configuration is adjusted.

## Resources

References used while designing and validating the project:
- [JUN's blog](https://jun-networks.hatenablog.com/entry/2022/12/05/234522)
- [RFC 9110, HTTP Semantics.](https://tex2e.github.io/rfc-translater/html/rfc9110.html)
- [RFC 9112, HTTP/1.1.](https://tex2e.github.io/rfc-translater/html/rfc9112.html)
- [RFC 6265, HTTP State Management Mechanism.](https://datatracker.ietf.org/doc/html/rfc6265)
- [What is HttpRequest HttpResponse?](Https://qiita.com/minateru/items/8693538bbd0768855266)
- [List of HTTP Status Codes](https://qiita.com/takuo_maeda/items/9cff0b03e74f8f600eee)
- The Linux manual pages for `socket`, `bind`, `listen`, `accept`, `epoll`,
  `fcntl`, `read`, `write`, `pipe`, `fork`, `dup2`, and `execve`.
- The CGI/1.1 environment variable conventions.
- nginx documentation and observed behavior for configuration structure,
  location matching, static file serving, redirects, and error pages.

AI assistance was used for review-oriented tasks: checking the project against
the subject requirements, identifying risk areas, discussing test strategies,
and drafting documentation. The implementation and final behavior were reviewed
and validated by the team.
