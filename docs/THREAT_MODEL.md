# Threat Model — STM32F407 Secure Boot

> Fill each section in your own words. Two worked examples are provided per section to
> show the level of detail; delete them once you've written your own. This document is
> the thing interviewers will read first.

**Target:** STM32F407VG (Cortex-M4, 1 MB flash, no crypto accelerator, no TrustZone)
**Author:** Anas Nizami
**Date:**
**Status:** draft

---

## 1. What am I protecting? (Assets)

What has value here — to me, and to an attacker?

| Asset | Why it matters | Impact if compromised |
| --- | --- | --- |
| Application firmware integrity | Device must run only code I authored | Attacker executes arbitrary code with full hardware access |
| Signing Private Key | Single most important thing that makes a signature unforgeable | An attacker can sign arbitrary firmware that every device accepts |
| Bootloader code | Root of trust — nothing verifies it | Attacker patches out the signature check. All crypto becomes inert |
| Monotonic rollback counter | Records highest version ever booted; provides freshness, which signatures cannot | Attacker downgrades to a genuinely-signed but known-vulnerable release |
| Debug interface (SWD) | Can alter execution by reading or writing the flash, RAM and CPU | Override the verification result. Results of failed verication can be altered to pass |
| Verification public key (bootloader flash) | Not secret, but must not be replaceable — it is the anchor every check is made against | Attacker substitutes their own public key and signs firmware the device will accept |

---

## 2. Who is attacking, and what can they do? (Adversaries)

Define capability levels — this is what bounds the whole design.

| Adversary | Capabilities | In scope? |
| --- | --- | --- |
| Remote attacker | Can send crafted firmware over the update channel; no physical access | Yes |
| Attacker with physical access | Can use SWD or other debub feature | Yes |
| Employee with correct credentials | Can access and modify the firmware during early development | No - keys and build systems need oganizational level control and security |
| Attacker with access to the device before consumer gets it | Someone with access during supply chain can falsh whatever they want. They can do it before  RDP and WRP are set | Yes |
| Skilled attacker with access to lab equipment | Can perform litching, power analysis, decapping, direct flash removal | No, no countermeasures exist in this design, and defending against it needs hardware support this part doesn't have.|

RDP - Read Out Protection : Controls whether external tools can read falsh or not. It has 3 levels.

1. Level 0 : No protection - Debugger reads and writes everything.
2. Level 1 : Falsh unreadable via a debugger - User can still connect SWD, but any attempt to read flash triggers a mass erase of the entire chip.
3. Level 2 : Debug interface permanently disabled - No SWD, no JTAG, no boot from RAM or system memory. Irreversible. There is no way back, ever.

WRP - Write Protection : Marks chosen flash sectors read-only at the hardware level

---

## 3. What attacks am I defending against?

For each: the attack, the mitigation, and which phase implements it.

| # | Attack | Mitigation | Phase |
| --- | --- | --- | --- |
| A1 | Attacker rewrites app flash with malicious image | Bootloader verifies ECDSA signature before execution; unsigned image refused | 3 |
| A2 | *(your turn)* | | |
| A3 | *(your turn)* | | |

Prompts: pushing a malicious image over UART. Reflashing an old, genuinely-signed but
vulnerable version. Overwriting the bootloader itself. Reading firmware out over SWD to
find secrets. Interrupting an update to leave the device in a half-written state.

---

## 4. What am I explicitly NOT defending against? (Out of scope)

**This is the most important section.** A threat model without stated boundaries is
marketing. For each, say *why* it's out of scope — cost, hardware limitation, or
acceptable risk.

| Out of scope | Why |
|---|---|
| Voltage/clock glitching to skip the signature check | No countermeasures on this part; would require redundant checks, randomized delays, and ideally hardware support. A determined attacker with physical access defeats this design. |
| *(your turn)* | |
| *(your turn)* | |

Prompts: side-channel analysis (power, EM). Decapping / invasive silicon attacks.
Compromise of the signing private key. Attacks on the build machine. Cold-boot RAM
attacks.

---

## 5. Root of trust — and its limits

Answer in prose:

- What is the root of trust in this design?
- What makes it trustworthy — physics, or policy?
- What single action by an attacker collapses it?
- How would this differ on an STM32H5/U5 with immutable ROM and TrustZone?

*(Your answer here. This paragraph is the one that demonstrates you understand secure
boot rather than just implemented it.)*

---

## 6. Assumptions

Every design rests on assumptions. List them so a reviewer can challenge them.

1. The signing private key is never exposed and never committed to version control.
2. *(your turn)*
3. *(your turn)*

Prompts: option bytes are correctly configured in production. RDP is enabled before
shipping. The build machine is not compromised. The public key baked into the bootloader
is the correct one.

---

## 7. Residual risk

After all mitigations, what risk remains and is knowingly accepted? One paragraph.

*(Your answer here.)*
