# Local playtest build validation

Prepared21 October2026. These are Linux cross-built playtest distributions of
Eidolon with Qt6.10.3. Source identity is in release-info.json; a commit date
spread requested by the owner does not represent the actual build date.

- Linux regression:180/180 Release checks passed in53.62 seconds, including
  numerical/compatibility comparisons, keyboard/layout/PDF tests and a forced
  Apple-style parser test against27 boundary cases and10,000 bit-pattern decimal
  roundtrips. Delivery unit tests passed6/6.
- Windows: real x64 PE executable compiled with MinGW GCC13.2;14 executable/DLL
  dependency records audited. All11 packaged module-entry startups passed under
  Wine11.20 with the shipped runtime. Recorded numerical golden comparisons
  passed under Wine for LP methods, changes/revised methods, Transportation,
  Assignment and Games, alongside backend/file checks.
- The first Wine harness passed86 of190 checks, including Python comparison/unit
  checks and11 packaged startups. Most GUI tests initially lacked the test-only
  Qt Test DLL. Adding that DLL and correcting test working directories exposed
  Wine fail-fast/timeouts; repetitive GUI retries were stopped and retained.
  This is not a full Windows GUI pass. The shipped app does not require Qt Test.
- macOS: real universal Mach-O executable and Qt framework/plugin closure built
  for arm64 and x86_64, with12 dependency records and both architectures checked.
  Minimum macOS version13.3; floating decimal parsing uses a C-locale fallback
  because this Apple libc++ lacks floating std::from_chars. The binary and
  friend check script have not been executed on a Mac.
- Neither target has native OS acceptance for this snapshot. Windows PowerShell
  friend checks also await a real Windows host. Both are unsigned/unnotarized
  playtest packages. Follow testing.md for platform startup and manual checks.

No GitHub Actions ran for these builds, and the repository remained private.
TORA is not included; its comparison must use your own installation. Matching
sample answers are useful evidence, not a proof covering all possible models.
