# Threat Model — STM32F407 Secure Boot

> Fill each section in your own words. Two worked examples are provided per section to
> show the level of detail; delete them once you've written your own. This document is
> the thing interviewers will read first.

**Target:** STM32F407VG (Cortex-M4, 1 MB flash, no crypto accelerator, no TrustZone)
**Author:** Anas Nizami
**Date:** 14th Aug 2026
**Status:** Completed

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
| Skilled attacker with access to lab equipment | Can perform litching, power analysis, decapping, direct flash removal | No, no countermeasures exist in this design, and defending against it needs hardware support this part doesn't have. |

---

## 3. What attacks am I defending against?

For each: the attack, the mitigation, and which phase implements it.

| # | Attack | Mitigation | Phase |
| --- | --- | --- | --- |
| A1 | Attacker rewrites app flash with malicious image | Bootloader verifies ECDSA signature before execution; unsigned image refused | 3 |
| A2 | Pushing a malicious image over UART | Received image is staged in slot B and its signature verified before the boot pointer is updated; a failing image is discarded and slot A remains active. | 5 (UART update path) |
| A3 | Reflashing with an older genuine version with vulnerability | A monotonic counter in sector 4 recording the highest version ever booted; images with a lower version are refused. | 4 (rollback counter) |
| A4 | Overwriting the bootloader itself | WRP on flash sectors 0–3 marks the bootloader read-only at hardware level; no code, including the update path, can erase or program it. | 4 (WRP) |
| A5 | Reading firmware out over SWD to find secrets | SET RDP to Level 1 | 4 (RDP) |
| A6 | Interrupting an update to leave the device in a half-written state | Divide storage into two separate slots. The device runs from Bank A while the installer writes the new update to Bank B. the running image is never erased until the new one is verified and activated | 5 (A/B slots) |

---

## 4. What am I explicitly NOT defending against? (Out of scope)

**This is the most important section.** A threat model without stated boundaries is
marketing. For each, say *why* it's out of scope — cost, hardware limitation, or
acceptable risk.

| Out of scope | Why |
| --- | --- |
| Voltage/clock glitching to skip the signature check | No countermeasures on this part; would require redundant checks, randomized delays, and ideally hardware support. A determined attacker with physical access defeats this design. |
| Side-channel analysis | Verification is not constant-time or power-balanced, and the F407 has no crypto accelerator or masking support. |
| Decapping / invasive silicon attacks | Requires specialist lab equipment and destroys the device. Cost to the attacker is disproportionate to the value of a single unit. |
| Compromise of the signing private key | Organizational control, not a device property. |
| Attacks on the build machine | Outside the trust boundary. If the signing environment is compromised, malicious firmware is signed legitimately. |
| Cold-boot RAM attacks | No long-lived secrets are held in RAM — only a public key and an image hash, both non-confidential. Nothing of value to recover. |

---

## 5. Root of trust — and its limits

The root of trust is the bootloader I programmed. The bootloader authenticates everything, but nothing authenticates the bootloader — making it the single point of trust.
Its trustworthiness depends on RDP and WRP being correctly configured, and there is always a possibility of those bits being changed. This is therefore policy protection, not hardware or silicon protection. As noted in the adversary table, an attacker with access to the device during the supply chain can modify these bits before they are set. The STM32F407 does contain a mask ROM, but it only boots — it performs no verification.
On an STM32H5 or U5, the immutable boot ROM verifies the bootloader's signature before executing it, anchoring the chain in silicon that cannot be modified after fabrication. This project has no equivalent.

---

## 6. Assumptions

Every design rests on assumptions. List them so a reviewer can challenge them.

1. The signing private key is never exposed and never committed to version control.
2. RDP is set to Level 1 and WRP is enabled in a controlled environment before device leaves manufacturing.
3. Build machine was not compromised. A malicious and valid signed firmware will be accepted by every design downstream.
4. Public key baked in the bootloader is the intended one.
5. micro-ecc's P-256 verification is correct as used, including signature and public key encoding.

---

## 7. Residual risk

After all mitigations, what risk remains and is knowingly accepted? One paragraph.

With all the mitigations in place, the design defends against remote attackers and opportunistic physical access. Unsigned or tampered firmware will not execute, downgrades to older signed releases are refused, and the bootloader cannot be overwritten by application code.
The remaining risks are known and accepted in the following areas - legitimate holder of the signing key or a compromised build environment. In this case signatures produced are valid by construction, and no on-device check can distinguish them. Second, an attacker with access during the provisioning window. The attacker can switch legitimate key with their own and after that subsequent verification passes against an attacker-controlled trust anchor. Third, an attacker with sustained physical access and specialist equipment. the STM32F407 provides no countermeasures against glitching or side-channel analysis, and mitigating these requires hardware support this part does not have.

## 8. Glossary

RDP - Read Out Protection : Controls whether external tools can read falsh or not. It has 3 levels.

1. Level 0 : No protection - Debugger reads and writes everything.
2. Level 1 : Falsh unreadable via a debugger - User can still connect SWD, but any attempt to read flash triggers a mass erase of the entire chip.
3. Level 2 : Debug interface permanently disabled - No SWD, no JTAG, no boot from RAM or system memory. Irreversible. There is no way back, ever.

WRP - Write Protection : Marks chosen flash sectors read-only at the hardware level
