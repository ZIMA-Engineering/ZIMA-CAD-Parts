# Password Manager Manual Verification

This repository now includes a small local fixture server for exercising the embedded browser password manager end to end.

## Start the fixture

Run the fixture from the repository root:

```bash
python3 tools/manual-tests/password_manager_fixture.py --port 18080
```

The default credentials are:

- Username: `alice`
- Password: `secret123`
- Username: `bob`
- Password: `secret456`
- HTTP Basic realm: `ZCP Test Realm`

## Fixture routes

- `/form-login`
  Standard username/password form that redirects to `/form-protected`.
- `/form-protected`
  Cookie-protected page shown after a successful form or SPA login and indicates
  which account is signed in.
- `/form-logout`
  Clears the fixture session cookie.
- `/spa-login`
  Real `<form>` handled through `preventDefault()` and `fetch()`.
- `/change-password`
  Signed-in helper page for changing the current account password so the
  update-password prompt can be tested.
- `/basic/`
  HTTP Basic authentication challenge with realm `ZCP Test Realm`.

## Manual checklist

### A. Shared profile persistence

1. Start the fixture server.
2. Log into `/form-login`.
3. Restart ZIMA-CAD-Parts.
4. Verify persistent site data survives the restart.
5. Verify local tech-spec pages still load correctly.
6. Verify downloads still use the existing download flow.

### B. HTTP auth save flow

1. Visit `/basic/`.
2. Enter the fixture credentials and enable the remember option.
3. Revisit `/basic/`.
4. Verify the dialog is prefilled from the saved record.
5. Open `Settings -> Browser -> Saved passwords...` and confirm the entry is listed.
6. Delete the entry from the manager dialog.
7. Revisit `/basic/` and verify the dialog is no longer prefilled.

### C. HTML save and autofill flow

1. Visit `/form-logout`.
2. Open `/form-login`.
3. Submit the fixture credentials.
4. Accept the save prompt.
5. Re-open `/form-login`.
6. Verify username and password autofill occur.
7. Open `/change-password`, set a new password, then submit the login form again and verify the update prompt appears.
8. Verify the saved-password manager can reveal, copy, and delete the stored entry.

### D. SPA-style submit flow

1. Open `/spa-login`.
2. Submit the fixture credentials.
3. Verify the save prompt still appears.
4. Revisit `/spa-login` and verify autofill still works.

### E. Local content safety

1. Open the built-in about page and local `file://.../0000-index/index.html` pages.
2. Verify the password manager does not prompt or autofill there.

### F. Settings persistence

1. Toggle the Browser settings.
2. Close and reopen the settings dialog.
3. Restart the application.
4. Verify the Browser settings persist.

### G. Multiple accounts on one form

1. Start the fixture.
2. Visit `/form-logout`.
3. Open `/form-login`.
4. Submit `alice` / `secret123`.
5. Accept the save prompt.
6. Log out and submit `bob` / `secret456` on the same `/form-login`.
7. Accept the save prompt.
8. Re-open `/form-login`.
9. Verify one account autofills by default.
10. Verify a visible multi-account chooser appears for the form.
11. Pick the other saved account and verify both username and password switch.
12. Clear the username field, type the other saved username manually, and
    verify the matching password autofills.
13. Type an unsaved username and verify a stale autofilled password is cleared.
