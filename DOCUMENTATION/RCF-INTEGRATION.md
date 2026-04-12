<!-- NOTICE: This file is protected under RCF-PL v1.2.8 -->
# RCF Integration Guide — Firmware Implementation

This document describes how the Restricted Correlation Framework (RCF) is integrated into this firmware codebase.

## 1. Compliance Markers

We use standard RCF markers to identify and protect proprietary logic.

### Block Protection (`@RCF-START` / `@RCF-END`)
Sensitive algorithms are wrapped in these markers to prevent automated extraction.
```c
// [RCF-START][M-SYNC-ALGO]
void secret_sync_logic() {
    // ... proprietary implementation ...
}
// [RCF-END]
```

### Visibility Notices (`[RCF:NOTICE]`)
Each file contains a visibility notice:
- **[RCF:PUBLIC]**: General purpose code, boilerplate, drivers.
- **[RCF:PROTECTED]**: Core methodologies, proprietary but auditable.
- **[RCF:RESTRICTED]**: Highly sensitive logic (e.g., vault management, key derivation).

## 2. Enforcement via GitLab CI

The `.gitlab-ci.yml` file ensures that every push is checked for RCF compliance. If a developer adds new logic without appropriate markers, the build will fail.

### CI Script
```yaml
rcf_compliance_check:
  script:
    - rcf-cli check .
```

## 3. Best Practices
- Always wrap new protocol features in `@RCF-START`.
- Use descriptive block IDs like `[M-AUTH-CHALLENGE]` for easier tracking in audit logs.
- Reference the official RCF specification at [rcf.aliyev.site](https://rcf.aliyev.site).

---
© 2026 Aladdin Aliyev. All rights reserved.
