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
|---|---|---|
| Application firmware integrity | Device must run only code I authored | Attacker executes arbitrary code with full hardware access |
| Signing Private Key | Single most important thing that makes a signature unforgeable | An attacker can sign arbitrary firmware that every device accepts |
| Bootloader code | Root of trust — nothing verifies it | Attacker patches out the signature check. All crypto becomes inert |
| Monotonic rollback counter | Records highest version ever booted; provides freshness, which signatures cannot | Attacker downgrades to a genuinely-signed but known-vulnerable release |
| Debug interface (SWD) | Can alter execution by reading or writing the flash, RAM and CPU | Override the verofocation result. Results of failed verication can be altered to pass |

---

## 2. Who is attacking, and what can they do? (Adversaries)

Define capability levels — this is what bounds the whole design.

| Adversary | Capabilities | In scope? |
|---|---|---|
| Remote attacker | Can send crafted firmware over the update channel; no physical access | Yes |
| *(your turn)* | | |
| *(your turn)* | | |

Prompts: attacker with brief physical access and an ST-Link. Attacker with unlimited
physical access and lab equipment. Malicious insider with the signing key. Supply-chain
attacker who modifies flash before you receive the board.

---

## 3. What attacks am I defending against?

For each: the attack, the mitigation, and which phase implements it.

| # | Attack | Mitigation | Phase |
|---|---|---|---|
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
