# Documentation Policy - Single File Per Peripheral

## 📋 Policy

**ONE master documentation file per peripheral driver.**

### Rules
1. ✅ **Create ONE comprehensive guide** per peripheral
2. ✅ **UPDATE existing file** instead of creating new ones
3. ✅ **Delete old fragmented files** to avoid confusion
4. ✅ **Keep documentation consolidated** and current

---

## 📁 Current Master Documentation Files

### GPS Driver
**File:** `GPS_COMPLETE_GUIDE.md`  
**Location:** Project root  
**Status:** ✅ Active (v2.2.0 - Bug Fix Release)

**Contents:**
- System Overview
- Features & Capabilities
- Hardware Setup
- Software Architecture
- Configuration (8 profiles)
- API Reference (17 functions)
- Troubleshooting (all known issues)
- Performance Metrics
- Known Issues & Fixes
- Testing & Verification

**Old Files Deleted:**
- ❌ PRODUCTION_GPS_COMPLETE.md
- ❌ GPS_MULTI_GNSS_FIX.md
- ❌ GPS_SUCCESS_SUMMARY.md
- ❌ GPS_CONSTELLATION_FIX.md
- ❌ GPS_CONSTELLATION_FIX_V2.md
- ❌ GPS_SATELLITES_IN_VIEW_FIX.md
- ❌ GPS_SATS_IN_VIEW_FINAL_FIX.md
- ❌ GPS_LOW_SATELLITE_COUNT_TROUBLESHOOTING.md
- ❌ GPS_BUGFIX_REPORT.md (Nov 11, 2025)
- ❌ GPS_QUICK_START.md (Nov 11, 2025)
- ❌ GPS_MULTI_GNSS_ENABLE.md (Nov 11, 2025)
- ❌ GPS_NEO_M8N_CONFIG.md (Nov 11, 2025)
- ❌ ECUAL/GPS/GPS_PRODUCTION_FEATURES.md
- ❌ ECUAL/GPS/GPS_IMPLEMENTATION_COMPLETE.md

---

## 🔄 Update Process

### When Making Changes
1. **Edit** `GPS_COMPLETE_GUIDE.md` (or relevant master file)
2. **Update** the relevant section
3. **Add** to "Version History" section
4. **DO NOT** create new documentation files

### When Adding New Features
1. **Update** "Features & Capabilities" section
2. **Update** "API Reference" section
3. **Add** to "Version History"
4. **Update** version number

### When Fixing Bugs
1. **Update** "Known Issues & Fixes" section
2. **Update** "Troubleshooting" section
3. **Add** to "Version History"
4. **Mark** issue as fixed with ✅

---

## 📝 Template for New Peripherals

When creating documentation for a new peripheral (e.g., SPI, CAN, ADC):

**File Name:** `[PERIPHERAL]_COMPLETE_GUIDE.md`  
**Location:** Project root

**Required Sections:**
1. System Overview
2. Features & Capabilities
3. Hardware Setup
4. Software Architecture
5. Configuration
6. API Reference
7. Troubleshooting
8. Performance Metrics
9. Known Issues & Fixes
10. Testing & Verification
11. Version History

---

## ✅ Benefits

### Single File Approach
- ✅ **Easy to find** - One place for all information
- ✅ **No confusion** - No duplicate or conflicting info
- ✅ **Always current** - Updates in one place
- ✅ **Complete context** - All history and fixes together
- ✅ **Better searchability** - Ctrl+F finds everything

### Multiple Files Approach (OLD - DON'T DO THIS)
- ❌ **Hard to find** - Which file has the info?
- ❌ **Confusion** - Conflicting information
- ❌ **Outdated** - Old files not updated
- ❌ **Fragmented** - Missing context
- ❌ **Poor searchability** - Must search multiple files

---

## 🎯 Summary

**Policy:** ONE master documentation file per peripheral.

**Current Files:**
- ✅ `GPS_COMPLETE_GUIDE.md` - GPS/NEO-M8N (v2.2.0)

**Action on Updates:**
- ✅ Edit existing master file
- ❌ Don't create new files

**This policy ensures clean, maintainable, and up-to-date documentation!**
