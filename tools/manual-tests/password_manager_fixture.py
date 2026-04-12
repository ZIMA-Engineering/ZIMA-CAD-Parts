#!/usr/bin/env python3

import argparse
import base64
import html
import json
from http import HTTPStatus
from http.cookies import SimpleCookie
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, quote, unquote, urlencode, urlsplit


REALM = "ZCP Test Realm"
SESSION_COOKIE_NAME = "zcp_session"


def page(title, body):
    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>{html.escape(title)}</title>
  <style>
    body {{
      font-family: sans-serif;
      margin: 2rem auto;
      max-width: 48rem;
      line-height: 1.5;
      padding: 0 1rem;
    }}
    form {{
      display: grid;
      gap: 0.75rem;
      margin: 1rem 0;
      max-width: 26rem;
    }}
    label {{
      display: grid;
      gap: 0.25rem;
      font-weight: 600;
    }}
    input, button {{
      font: inherit;
      padding: 0.5rem 0.65rem;
    }}
    nav {{
      display: flex;
      flex-wrap: wrap;
      gap: 0.75rem;
      margin-bottom: 1rem;
    }}
    .message {{
      background: #f5f5f5;
      border: 1px solid #d0d0d0;
      padding: 0.75rem;
      margin: 1rem 0;
    }}
    .error {{
      border-color: #cc6666;
      background: #fff0f0;
    }}
    code {{
      background: #f5f5f5;
      padding: 0.1rem 0.25rem;
    }}
  </style>
</head>
<body>
{body}
</body>
</html>
"""


class FixtureState:
    def __init__(self, accounts):
        self.accounts = dict(accounts)

    def credential_items(self):
        return sorted(self.accounts.items())

    def is_valid_login(self, username, password):
        return self.accounts.get(username) == password

    def update_password(self, username, password):
        if username in self.accounts:
            self.accounts[username] = password


class PasswordManagerFixtureHandler(BaseHTTPRequestHandler):
    server_version = "ZCPPasswordManagerFixture/1.0"

    @property
    def state(self):
        return self.server.fixture_state

    def do_GET(self):
        parsed = urlsplit(self.path)
        path = parsed.path
        query = parse_qs(parsed.query, keep_blank_values=True)

        if path == "/":
            self.render_index()
            return
        if path == "/form-login":
            self.render_form_login(query)
            return
        if path == "/form-protected":
            self.render_form_protected()
            return
        if path == "/form-logout":
            self.render_form_logout()
            return
        if path == "/change-password":
            self.render_change_password(query)
            return
        if path == "/spa-login":
            self.render_spa_login()
            return
        if path == "/basic/":
            self.render_basic_auth()
            return

        self.send_error(HTTPStatus.NOT_FOUND, "Not found")

    def do_POST(self):
        parsed = urlsplit(self.path)
        path = parsed.path
        form = self.read_form()

        if path == "/form-login":
            self.handle_form_login(form)
            return
        if path == "/api/login":
            self.handle_api_login(form)
            return
        if path == "/change-password":
            self.handle_change_password(form)
            return

        self.send_error(HTTPStatus.NOT_FOUND, "Not found")

    def read_form(self):
        length = int(self.headers.get("Content-Length", "0"))
        payload = self.rfile.read(length).decode("utf-8") if length else ""
        return {key: values[0] for key, values in parse_qs(payload, keep_blank_values=True).items()}

    def session_username(self):
        cookie_header = self.headers.get("Cookie", "")
        if not cookie_header:
            return None

        cookie = SimpleCookie()
        cookie.load(cookie_header)
        morsel = cookie.get(SESSION_COOKIE_NAME)
        if morsel is None or not morsel.value:
            return None

        username = unquote(morsel.value)
        return username if username in self.state.accounts else None

    def basic_auth_username(self):
        header = self.headers.get("Authorization", "")
        if not header.startswith("Basic "):
            return None

        try:
            decoded = base64.b64decode(header[6:]).decode("utf-8")
        except Exception:
            return None

        username, _, password = decoded.partition(":")
        return username if self.state.is_valid_login(username, password) else None

    def session_cookie_header(self, username):
        encoded_username = quote(username, safe="")
        return f"{SESSION_COOKIE_NAME}={encoded_username}; Path=/; HttpOnly; SameSite=Lax"

    def expired_session_cookie_header(self):
        return f"{SESSION_COOKIE_NAME}=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax"

    def send_html(self, status, body, extra_headers=None):
        encoded = body.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded)))
        if extra_headers:
            for key, value in extra_headers:
                self.send_header(key, value)
        self.end_headers()
        self.wfile.write(encoded)

    def send_json(self, status, payload, extra_headers=None):
        encoded = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded)))
        if extra_headers:
            for key, value in extra_headers:
                self.send_header(key, value)
        self.end_headers()
        self.wfile.write(encoded)

    def redirect(self, location, extra_headers=None):
        self.send_response(HTTPStatus.SEE_OTHER)
        self.send_header("Location", location)
        if extra_headers:
            for key, value in extra_headers:
                self.send_header(key, value)
        self.end_headers()

    def render_index(self):
        credentials = "\n".join(
            f"  <li><code>{html.escape(username)}</code> / "
            f"<code>{html.escape(password)}</code></li>"
            for username, password in self.state.credential_items()
        )
        body = page(
            "ZCP password manager fixture",
            f"""
<h1>ZCP password manager fixture</h1>
<p>Default credentials:</p>
<ul>
{credentials}
</ul>
<nav>
  <a href="/form-login">Form login</a>
  <a href="/spa-login">SPA login</a>
  <a href="/basic/">HTTP Basic auth</a>
  <a href="/form-protected">Protected page</a>
  <a href="/form-logout">Log out</a>
</nav>
<p>Use <a href="/change-password">Change password</a> after signing in to trigger the password-update flow.</p>
""",
        )
        self.send_html(HTTPStatus.OK, body)

    def render_form_login(self, query):
        message = ""
        if query.get("error"):
            message = '<p class="message error">Invalid username or password.</p>'

        body = page(
            "Form login",
            f"""
<h1>Form login</h1>
<nav>
  <a href="/">Home</a>
  <a href="/spa-login">SPA login</a>
  <a href="/basic/">HTTP Basic auth</a>
</nav>
{message}
<form action="/form-login" method="post">
  <label>
    Username
    <input type="text" name="username" autocomplete="username">
  </label>
  <label>
    Password
    <input type="password" name="password" autocomplete="current-password">
  </label>
  <button type="submit">Sign in</button>
</form>
""",
        )
        self.send_html(HTTPStatus.OK, body)

    def render_form_protected(self):
        username = self.session_username()
        if not username:
            self.redirect("/form-login")
            return

        body = page(
            "Protected page",
            f"""
<h1>Logged in</h1>
<p>You are signed in as <strong>{html.escape(username)}</strong>.</p>
<nav>
  <a href="/">Home</a>
  <a href="/change-password">Change password</a>
  <a href="/form-logout">Log out</a>
  <a href="/basic/">HTTP Basic auth</a>
</nav>
""",
        )
        self.send_html(HTTPStatus.OK, body)

    def render_form_logout(self):
        body = page(
            "Logged out",
            """
<h1>Logged out</h1>
<p>The fixture session cookie has been cleared.</p>
<nav>
  <a href="/form-login">Back to form login</a>
  <a href="/">Home</a>
</nav>
""",
        )
        self.send_html(
            HTTPStatus.OK,
            body,
            extra_headers=[("Set-Cookie", self.expired_session_cookie_header())],
        )

    def render_change_password(self, query):
        username = self.session_username()
        if not username:
            self.redirect("/form-login")
            return

        message = ""
        if query.get("updated"):
            message = (
                '<p class="message">Password updated for '
                f'<code>{html.escape(username)}</code>.</p>'
            )

        body = page(
            "Change password",
            f"""
<h1>Change password</h1>
<nav>
  <a href="/form-protected">Protected page</a>
  <a href="/form-login">Form login</a>
  <a href="/form-logout">Log out</a>
</nav>
{message}
<p>Current signed-in username: <code>{html.escape(username)}</code></p>
<form action="/change-password" method="post">
  <label>
    New password
    <input type="password" name="new_password" autocomplete="new-password">
  </label>
  <button type="submit">Update password</button>
</form>
""",
        )
        self.send_html(HTTPStatus.OK, body)

    def render_spa_login(self):
        body = page(
            "SPA login",
            """
<h1>SPA login</h1>
<nav>
  <a href="/">Home</a>
  <a href="/form-login">Form login</a>
  <a href="/basic/">HTTP Basic auth</a>
</nav>
<p id="status" class="message" hidden></p>
<form id="spa-login-form" action="/api/login" method="post">
  <label>
    Username
    <input type="text" name="username" autocomplete="username">
  </label>
  <label>
    Password
    <input type="password" name="password" autocomplete="current-password">
  </label>
  <button type="submit">Sign in with fetch()</button>
</form>
<script>
  const form = document.getElementById("spa-login-form");
  const status = document.getElementById("status");

  form.addEventListener("submit", async function (event) {
    event.preventDefault();

    const body = new URLSearchParams(new FormData(form));
    const response = await fetch(form.action, {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: body.toString()
    });

    if (response.ok) {
      window.location.href = "/form-protected";
      return;
    }

    status.hidden = false;
    status.className = "message error";
    status.textContent = "Invalid username or password.";
  });
</script>
""",
        )
        self.send_html(HTTPStatus.OK, body)

    def render_basic_auth(self):
        username = self.basic_auth_username()
        if not username:
            self.send_response(HTTPStatus.UNAUTHORIZED)
            self.send_header("WWW-Authenticate", f'Basic realm="{REALM}"')
            self.end_headers()
            return

        body = page(
            "HTTP Basic auth",
            f"""
<h1>HTTP Basic authentication success</h1>
<p>Authenticated as <strong>{html.escape(username)}</strong> for realm <code>{REALM}</code>.</p>
<nav>
  <a href="/">Home</a>
  <a href="/form-login">Form login</a>
</nav>
""",
        )
        self.send_html(HTTPStatus.OK, body)

    def handle_form_login(self, form):
        username = form.get("username", "")
        password = form.get("password", "")

        if self.state.is_valid_login(username, password):
            self.redirect(
                "/form-protected",
                extra_headers=[("Set-Cookie", self.session_cookie_header(username))],
            )
            return

        query = urlencode({"error": "1"})
        self.redirect(f"/form-login?{query}")

    def handle_api_login(self, form):
        username = form.get("username", "")
        password = form.get("password", "")

        if self.state.is_valid_login(username, password):
            self.send_json(
                HTTPStatus.OK,
                {"ok": True, "username": username},
                extra_headers=[("Set-Cookie", self.session_cookie_header(username))],
            )
            return

        self.send_json(HTTPStatus.UNAUTHORIZED, {"ok": False})

    def handle_change_password(self, form):
        username = self.session_username()
        if not username:
            self.redirect("/form-login")
            return

        new_password = form.get("new_password", "").strip()
        if not new_password:
            self.redirect("/change-password")
            return

        self.state.update_password(username, new_password)
        self.redirect("/change-password?updated=1")


def main():
    parser = argparse.ArgumentParser(description="Manual password-manager test fixture for ZIMA-CAD-Parts")
    parser.add_argument("--host", default="127.0.0.1", help="Host interface to bind")
    parser.add_argument("--port", type=int, default=18080, help="Port to bind")
    parser.add_argument("--username", default="alice", help="Primary fixture username")
    parser.add_argument("--password", default="secret123", help="Primary fixture password")
    parser.add_argument("--second-username", default="bob", help="Second fixture username")
    parser.add_argument("--second-password", default="secret456", help="Second fixture password")
    args = parser.parse_args()

    server = ThreadingHTTPServer((args.host, args.port), PasswordManagerFixtureHandler)
    accounts = {
        args.username: args.password,
    }
    if args.second_username:
        accounts[args.second_username] = args.second_password
    server.fixture_state = FixtureState(accounts)

    print(f"Serving password-manager fixture on http://{args.host}:{args.port}")
    print("Default credentials:")
    for username, password in server.fixture_state.credential_items():
        print(f"  {username} / {password}")
    print(f"HTTP Basic auth realm: {REALM}")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down fixture server.")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
