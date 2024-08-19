/*
 * Wazuh Vulnerability Scanner - Unit Tests
 * Copyright (C) 2015, Wazuh Inc.
 * September 21, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "scanContext_test.hpp"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_deltas_generated.h"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_deltas_schema.h"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_synchronization_generated.h"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_synchronization_schema.h"
#include "../scanOrchestrator/scanContext.hpp"
#include "MockGlobalData.hpp"
#include "MockOsDataCache.hpp"
#include "TrampolineGlobalData.hpp"
#include "TrampolineOsDataCache.hpp"
#include "TrampolineRemediationDataCache.hpp"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "json.hpp"

using ::testing::_;
using scanContext_t = TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>;

std::shared_ptr<MockGlobalData> spGlobalDataMock;

namespace NSScanContextTest
{
const std::string DELTA_PACKAGES_INSERTED_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "dbsync_packages",
                "data": {
                    "architecture": "amd64",
                    "checksum": "1e6ce14f97f57d1bbd46ff8e5d3e133171a1bbce",
                    "description": "library for GIF images library",
                    "format": "deb",
                    "groups": "libs",
                    "item_id": "ec465b7eb5fa011a336e95614072e4c7f1a65a53",
                    "multiarch": "same",
                    "name": "libgif7",
                    "priority": "optional",
                    "scan_time": "2023/08/04 19:56:11",
                    "size": 72,
                    "source": "giflib",
                    "vendor": "Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>",
                    "version": "5.1.9-1"
                },
                "operation": "INSERTED"
            }
        )";

const std::string DELTA_PACKAGES_DELETED_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "dbsync_packages",
                "data": {
                    "architecture": "amd64",
                    "checksum": "1e6ce14f97f57d1bbd46ff8e5d3e133171a1bbce",
                    "description": "library for GIF images library",
                    "format": "deb",
                    "groups": "libs",
                    "item_id": "ec465b7eb5fa011a336e95614072e4c7f1a65a53",
                    "multiarch": "same",
                    "name": "libgif7",
                    "priority": "optional",
                    "scan_time": "2023/08/04 19:56:11",
                    "size": 72,
                    "source": "giflib",
                    "vendor": "Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>",
                    "version": "5.1.9-1"
                },
                "operation": "DELETED"
            }
        )";

const std::string DELTA_OSINFO_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "dbsync_osinfo",
                "data": {
                    "architecture":"x86_64",
                    "checksum":"1691178971959743855",
                    "hostname":"redhat",
                    "os_codename":"9",
                    "os_major":"9",
                    "os_minor":"0",
                    "os_name":"Redhat",
                    "os_patch":"6",
                    "os_platform":"rhel",
                    "os_version":"9.0.6",
                    "release":"5.4.0-155-generic",
                    "scan_time":"2023/08/04 19:56:11",
                    "sysname":"Linux",
                    "version":"#172-Ubuntu SMP Fri Jul 7 16:10:02 UTC 2023"
                },
                "operation": "INSERTED"
            }
        )";

const std::string DELTA_HOTFIXES_INSERTED_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "dbsync_hotfixes",
                "data": {
                    "checksum":"56162cd7bb632b4728ec868e8e271b01222ff131",
                    "hotfix":"KB12345678",
                    "scan_time":"2024/01/05 10:30:25"
                },
                "operation": "INSERTED"
            }
        )";

const std::string DELTA_HOTFIXES_DELETED_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "dbsync_hotfixes",
                "data": {
                    "checksum":"56162cd7bb632b4728ec868e8e271b01222ff131",
                    "hotfix":"KB12345678",
                    "scan_time":"2024/01/05 10:30:25"
                },
                "operation": "DELETED"
            }
        )";

const std::string DELTA_NO_OPERATION_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "dbsync_hotfixes",
                "data": {
                    "checksum":"56162cd7bb632b4728ec868e8e271b01222ff131",
                    "hotfix":"KB12345678"
                }
            }
        )";

const std::string SYNCHRONIZATION_STATE_OSINFO_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "state",
                "data": {
                    "attributes_type": "syscollector_osinfo",
                    attributes: {
                        checksum: "1691513227478039559",
                        hostname: "Supercomputer",
                        os_codename: "focal",
                        os_major: "200",
                        os_minor: "000",
                        os_name: "Wazuh OS",
                        os_patch: "2",
                        os_platform: "bsd",
                        os_version: "200.000.2",
                        release: "5.4.0-153-generic",
                        scan_time: "0000/00/00 00:00:00",
                        sysname: "Linux",
                        version: "#170-WazuhOS SMP Fri Jun 16 13:43:31 UTC 2023"
                    },
                    index: "WazuhOS",
                    timestamp: ""
                }
            }
        )";

const std::string SYNCHRONIZATION_STATE_PACKAGES_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "state",
                "data": {
                    "attributes_type": "syscollector_packages",
                    "attributes": {
                        "architecture": " ",
                        "checksum": "d24e16553ac8f6983f16fb7a68b841ac8876d745",
                        "description": " ",
                        "format": "pypi",
                        "groups": " ",
                        "install_time": " ",
                        "item_id": "e051a99ad3950084b538c6538c0fcad0f1c4c713",
                        "location": "/usr/lib/python3/dist-packages/language_selector-0.1.egg-info/PKG-INFO",
                        "name": "language-selector",
                        "priority": " ",
                        "scan_time": "2023/12/01 20:48:21",
                        "size": 0,
                        "source": " ",
                        "vendor": " ",
                        "version": "0.1"
                    },
                    "index": "e051a99ad3950084b538c6538c0fcad0f1c4c713",
                    "timestamp": ""
                }
            }
        )";

const std::string SYNCHRONIZATION_STATE_HOTFIXES_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "state",
                "data": {
                    "attributes_type": "syscollector_hotfixes",
                    "attributes": {
                        "checksum":"56162cd7bb632b4728ec868e8e271b01222ff131",
                        "hotfix":"KB12345678",
                        "scan_time":"2024/01/05 10:30:25"
                    },
                    "index": "e051a99ad3950084b538c6538c0fcad0f1c4c713",
                    "timestamp": ""
                }
            }
        )";

const std::string SYNCHRONIZATION_INTEGRITY_CLEAR_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal"
                },
                "data_type": "integrity_clear",
                "data": {
                    "id": 1700236640,
                    "attributes_type": "syscollector_packages"
                }
            }
        )";
} // namespace NSScanContextTest

using namespace NSScanContextTest;

void ScanContextTest::TearDown()
{
    spGlobalDataMock.reset();
    spOsDataCacheMock.reset();
    spRemediationDataCacheMock.reset();
}

const char* ScanContextTest::fbStringGetHelper(const flatbuffers::String* pStr)
{
    return pStr ? pStr->c_str() : "";
}

TEST_F(ScanContextTest, TestSyscollectorDeltasPackagesInserted)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "osdata_codeName",
               .majorVersion = "osdata_majorVersion",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "osdata_platform",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillOnce(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::PackageInsert);

    EXPECT_STREQ(scanContext->agentId().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(scanContext->agentIp().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_name()));
    EXPECT_STREQ(
        scanContext->agentVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_version()));
    EXPECT_STREQ(
        scanContext->packageName().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->name()));
    EXPECT_STREQ(
        scanContext->packageVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->version()));
    EXPECT_STREQ(
        scanContext->packageVendor().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->vendor()));
    EXPECT_STREQ(scanContext->packageInstallTime().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->install_time()));
    EXPECT_STREQ(scanContext->packageLocation().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->location()));
    EXPECT_STREQ(scanContext->packageArchitecture().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->architecture()));
    EXPECT_STREQ(
        scanContext->packageGroups().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->groups()));
    EXPECT_STREQ(scanContext->packageDescription().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->description()));
    EXPECT_EQ(scanContext->packageSize(),
              SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->size());
    EXPECT_STREQ(scanContext->packagePriority().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->priority()));
    EXPECT_STREQ(scanContext->packageMultiarch().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->multiarch()));
    EXPECT_STREQ(
        scanContext->packageSource().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->source()));
    EXPECT_STREQ(
        scanContext->packageFormat().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->format()));
    EXPECT_STREQ(
        scanContext->packageItemId().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->item_id()));

    EXPECT_STREQ(scanContext->osHostName().data(), osData.hostName.c_str());
    EXPECT_STREQ(scanContext->osArchitecture().data(), osData.architecture.c_str());
    EXPECT_STREQ(scanContext->osName().data(), osData.name.c_str());
    EXPECT_STREQ(scanContext->osVersion().data(), osData.version.c_str());
    EXPECT_STREQ(scanContext->osCodeName().data(), osData.codeName.c_str());
    EXPECT_STREQ(scanContext->osMajorVersion().data(), osData.majorVersion.c_str());
    EXPECT_STREQ(scanContext->osMinorVersion().data(), osData.minorVersion.c_str());
    EXPECT_STREQ(scanContext->osPatch().data(), osData.patch.c_str());
    EXPECT_STREQ(scanContext->osBuild().data(), osData.build.c_str());
    EXPECT_STREQ(scanContext->osPlatform().data(), osData.platform.c_str());
    EXPECT_STREQ(scanContext->osKernelSysName().data(), osData.sysName.c_str());
    EXPECT_STREQ(scanContext->osKernelRelease().data(), osData.kernelRelease.c_str());
    EXPECT_STREQ(scanContext->osKernelVersion().data(), osData.kernelVersion.c_str());
    EXPECT_STREQ(scanContext->osRelease().data(), osData.release.c_str());
    EXPECT_STREQ(scanContext->osDisplayVersion().data(), osData.displayVersion.c_str());
}

TEST_F(ScanContextTest, TestSyscollectorDeltasPackagesDeleted)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "osdata_codeName",
               .majorVersion = "osdata_majorVersion",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "osdata_platform",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillOnce(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_DELETED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::PackageDelete);

    EXPECT_STREQ(scanContext->agentId().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(scanContext->agentIp().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_name()));
    EXPECT_STREQ(
        scanContext->agentVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_version()));
    EXPECT_STREQ(
        scanContext->packageName().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->name()));
    EXPECT_STREQ(
        scanContext->packageVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->version()));
    EXPECT_STREQ(
        scanContext->packageVendor().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->vendor()));
    EXPECT_STREQ(scanContext->packageInstallTime().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->install_time()));
    EXPECT_STREQ(scanContext->packageLocation().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->location()));
    EXPECT_STREQ(scanContext->packageArchitecture().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->architecture()));
    EXPECT_STREQ(
        scanContext->packageGroups().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->groups()));
    EXPECT_STREQ(scanContext->packageDescription().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->description()));
    EXPECT_EQ(scanContext->packageSize(),
              SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->size());
    EXPECT_STREQ(scanContext->packagePriority().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->priority()));
    EXPECT_STREQ(scanContext->packageMultiarch().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_packages()
                                       ->multiarch()));
    EXPECT_STREQ(
        scanContext->packageSource().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->source()));
    EXPECT_STREQ(
        scanContext->packageFormat().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->format()));
    EXPECT_STREQ(
        scanContext->packageItemId().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_packages()->item_id()));

    EXPECT_STREQ(scanContext->osHostName().data(), osData.hostName.c_str());
    EXPECT_STREQ(scanContext->osArchitecture().data(), osData.architecture.c_str());
    EXPECT_STREQ(scanContext->osName().data(), osData.name.c_str());
    EXPECT_STREQ(scanContext->osVersion().data(), osData.version.c_str());
    EXPECT_STREQ(scanContext->osCodeName().data(), osData.codeName.c_str());
    EXPECT_STREQ(scanContext->osMajorVersion().data(), osData.majorVersion.c_str());
    EXPECT_STREQ(scanContext->osMinorVersion().data(), osData.minorVersion.c_str());
    EXPECT_STREQ(scanContext->osPatch().data(), osData.patch.c_str());
    EXPECT_STREQ(scanContext->osBuild().data(), osData.build.c_str());
    EXPECT_STREQ(scanContext->osPlatform().data(), osData.platform.c_str());
    EXPECT_STREQ(scanContext->osKernelSysName().data(), osData.sysName.c_str());
    EXPECT_STREQ(scanContext->osKernelRelease().data(), osData.kernelRelease.c_str());
    EXPECT_STREQ(scanContext->osKernelVersion().data(), osData.kernelVersion.c_str());
    EXPECT_STREQ(scanContext->osRelease().data(), osData.release.c_str());
    EXPECT_STREQ(scanContext->osDisplayVersion().data(), osData.displayVersion.c_str());
}

TEST_F(ScanContextTest, TestSyscollectorDeltasOsInfo)
{
    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_OSINFO_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::Os);

    EXPECT_STREQ(scanContext->agentId().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(scanContext->agentIp().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_name()));
    EXPECT_STREQ(
        scanContext->agentVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_version()));
    EXPECT_STREQ(
        scanContext->osHostName().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->hostname()));
    EXPECT_STREQ(scanContext->osArchitecture().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_osinfo()
                                       ->architecture()));
    EXPECT_STREQ(
        scanContext->osName().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->os_name()));
    EXPECT_STREQ(scanContext->osVersion().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_osinfo()
                                       ->os_version()));
    EXPECT_STREQ(scanContext->osCodeName().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_osinfo()
                                       ->os_codename()));
    EXPECT_STREQ(
        scanContext->osMajorVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->os_major()));
    EXPECT_STREQ(
        scanContext->osMinorVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->os_minor()));
    EXPECT_STREQ(
        scanContext->osPatch().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->os_patch()));
    EXPECT_STREQ(
        scanContext->osBuild().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->os_build()));
    EXPECT_STREQ(scanContext->osPlatform().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_osinfo()
                                       ->os_platform()));
    EXPECT_STREQ(
        scanContext->osKernelSysName().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->sysname()));
    EXPECT_STREQ(
        scanContext->osKernelRelease().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->release()));
    EXPECT_STREQ(
        scanContext->osKernelVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->data_as_dbsync_osinfo()->version()));
    EXPECT_STREQ(scanContext->osRelease().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_osinfo()
                                       ->os_release()));
    EXPECT_STREQ(scanContext->osDisplayVersion().data(),
                 fbStringGetHelper(SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))
                                       ->data_as_dbsync_osinfo()
                                       ->os_display_version()));
}

TEST_F(ScanContextTest, TestSyscollectorDeltasHotfixesInserted)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "osdata_codeName",
               .majorVersion = "osdata_majorVersion",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "osdata_platform",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillOnce(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));
    EXPECT_CALL(*spRemediationDataCacheMock, addRemediationData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_HOTFIXES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::HotfixInsert);

    EXPECT_STREQ(scanContext->agentId().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(scanContext->agentIp().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_name()));
    EXPECT_STREQ(
        scanContext->agentVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_version()));

    EXPECT_STREQ(scanContext->osHostName().data(), osData.hostName.c_str());
    EXPECT_STREQ(scanContext->osArchitecture().data(), osData.architecture.c_str());
    EXPECT_STREQ(scanContext->osName().data(), osData.name.c_str());
    EXPECT_STREQ(scanContext->osVersion().data(), osData.version.c_str());
    EXPECT_STREQ(scanContext->osCodeName().data(), osData.codeName.c_str());
    EXPECT_STREQ(scanContext->osMajorVersion().data(), osData.majorVersion.c_str());
    EXPECT_STREQ(scanContext->osMinorVersion().data(), osData.minorVersion.c_str());
    EXPECT_STREQ(scanContext->osPatch().data(), osData.patch.c_str());
    EXPECT_STREQ(scanContext->osBuild().data(), osData.build.c_str());
    EXPECT_STREQ(scanContext->osPlatform().data(), osData.platform.c_str());
    EXPECT_STREQ(scanContext->osKernelSysName().data(), osData.sysName.c_str());
    EXPECT_STREQ(scanContext->osKernelRelease().data(), osData.kernelRelease.c_str());
    EXPECT_STREQ(scanContext->osKernelVersion().data(), osData.kernelVersion.c_str());
    EXPECT_STREQ(scanContext->osRelease().data(), osData.release.c_str());
    EXPECT_STREQ(scanContext->osDisplayVersion().data(), osData.displayVersion.c_str());
}

TEST_F(ScanContextTest, TestSyscollectorDeltasHotfixesDeleted)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "osdata_codeName",
               .majorVersion = "osdata_majorVersion",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "osdata_platform",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillOnce(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_HOTFIXES_DELETED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::HotfixDelete);

    EXPECT_STREQ(scanContext->agentId().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(scanContext->agentIp().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(
                     SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_name()));
    EXPECT_STREQ(
        scanContext->agentVersion().data(),
        fbStringGetHelper(
            SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer))->agent_info()->agent_version()));

    EXPECT_STREQ(scanContext->osHostName().data(), osData.hostName.c_str());
    EXPECT_STREQ(scanContext->osArchitecture().data(), osData.architecture.c_str());
    EXPECT_STREQ(scanContext->osName().data(), osData.name.c_str());
    EXPECT_STREQ(scanContext->osVersion().data(), osData.version.c_str());
    EXPECT_STREQ(scanContext->osCodeName().data(), osData.codeName.c_str());
    EXPECT_STREQ(scanContext->osMajorVersion().data(), osData.majorVersion.c_str());
    EXPECT_STREQ(scanContext->osMinorVersion().data(), osData.minorVersion.c_str());
    EXPECT_STREQ(scanContext->osPatch().data(), osData.patch.c_str());
    EXPECT_STREQ(scanContext->osBuild().data(), osData.build.c_str());
    EXPECT_STREQ(scanContext->osPlatform().data(), osData.platform.c_str());
    EXPECT_STREQ(scanContext->osKernelSysName().data(), osData.sysName.c_str());
    EXPECT_STREQ(scanContext->osKernelRelease().data(), osData.kernelRelease.c_str());
    EXPECT_STREQ(scanContext->osKernelVersion().data(), osData.kernelVersion.c_str());
    EXPECT_STREQ(scanContext->osRelease().data(), osData.release.c_str());
    EXPECT_STREQ(scanContext->osDisplayVersion().data(), osData.displayVersion.c_str());
}

TEST_F(ScanContextTest, TestSyscollectorDeltasNoOperation)
{
    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_NO_OPERATION_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));

    EXPECT_THROW(std::make_shared<scanContext_t>(msg), std::runtime_error);
}

TEST_F(ScanContextTest, TestSyscollectorSynchronizationStateOsInfo)
{
    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(SYNCHRONIZATION_STATE_OSINFO_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::Os);

    EXPECT_STREQ(
        scanContext->agentId().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(
        scanContext->agentIp().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_name()));
    EXPECT_STREQ(scanContext->agentVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_version()));
    EXPECT_STREQ(scanContext->osHostName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->hostname()));
    EXPECT_STREQ(scanContext->osArchitecture().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->architecture()));
    EXPECT_STREQ(scanContext->osName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_name()));
    EXPECT_STREQ(scanContext->osVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_version()));
    EXPECT_STREQ(scanContext->osCodeName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_codename()));
    EXPECT_STREQ(scanContext->osMajorVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_major()));
    EXPECT_STREQ(scanContext->osMinorVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_minor()));
    EXPECT_STREQ(scanContext->osPatch().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_patch()));
    EXPECT_STREQ(scanContext->osBuild().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_build()));
    EXPECT_STREQ(scanContext->osPlatform().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_platform()));
    EXPECT_STREQ(scanContext->osKernelSysName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->sysname()));
    EXPECT_STREQ(scanContext->osKernelRelease().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->release()));
    EXPECT_STREQ(scanContext->osKernelVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->version()));
    EXPECT_STREQ(scanContext->osRelease().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_release()));
    EXPECT_STREQ(scanContext->osDisplayVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_osinfo()
                                       ->os_display_version()));
}

TEST_F(ScanContextTest, TestSyscollectorSynchronizationStatePackages)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "osdata_codeName",
               .majorVersion = "osdata_majorVersion",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "osdata_platform",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillOnce(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(SYNCHRONIZATION_STATE_PACKAGES_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::PackageInsert);

    EXPECT_STREQ(
        scanContext->agentId().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(
        scanContext->agentIp().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_name()));
    EXPECT_STREQ(scanContext->agentVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_version()));
    EXPECT_STREQ(scanContext->packageName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->name()));
    EXPECT_STREQ(scanContext->packageVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->version()));
    EXPECT_STREQ(scanContext->packageVendor().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->vendor()));
    EXPECT_STREQ(scanContext->packageInstallTime().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->install_time()));
    EXPECT_STREQ(scanContext->packageLocation().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->location()));
    EXPECT_STREQ(scanContext->packageArchitecture().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->architecture()));
    EXPECT_STREQ(scanContext->packageGroups().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->groups()));
    EXPECT_STREQ(scanContext->packageDescription().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->description()));
    EXPECT_EQ(scanContext->packageSize(),
              SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                  ->data_as_state()
                  ->attributes_as_syscollector_packages()
                  ->size());
    EXPECT_STREQ(scanContext->packagePriority().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->priority()));
    EXPECT_STREQ(scanContext->packageMultiarch().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->multiarch()));
    EXPECT_STREQ(scanContext->packageSource().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->source()));
    EXPECT_STREQ(scanContext->packageFormat().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->format()));
    EXPECT_STREQ(scanContext->packageItemId().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->data_as_state()
                                       ->attributes_as_syscollector_packages()
                                       ->item_id()));

    EXPECT_STREQ(scanContext->osHostName().data(), osData.hostName.c_str());
    EXPECT_STREQ(scanContext->osArchitecture().data(), osData.architecture.c_str());
    EXPECT_STREQ(scanContext->osName().data(), osData.name.c_str());
    EXPECT_STREQ(scanContext->osVersion().data(), osData.version.c_str());
    EXPECT_STREQ(scanContext->osCodeName().data(), osData.codeName.c_str());
    EXPECT_STREQ(scanContext->osMajorVersion().data(), osData.majorVersion.c_str());
    EXPECT_STREQ(scanContext->osMinorVersion().data(), osData.minorVersion.c_str());
    EXPECT_STREQ(scanContext->osPatch().data(), osData.patch.c_str());
    EXPECT_STREQ(scanContext->osBuild().data(), osData.build.c_str());
    EXPECT_STREQ(scanContext->osPlatform().data(), osData.platform.c_str());
    EXPECT_STREQ(scanContext->osKernelSysName().data(), osData.sysName.c_str());
    EXPECT_STREQ(scanContext->osKernelRelease().data(), osData.kernelRelease.c_str());
    EXPECT_STREQ(scanContext->osKernelVersion().data(), osData.kernelVersion.c_str());
    EXPECT_STREQ(scanContext->osRelease().data(), osData.release.c_str());
    EXPECT_STREQ(scanContext->osDisplayVersion().data(), osData.displayVersion.c_str());
}

TEST_F(ScanContextTest, TestSyscollectorSynchronizationStateHotfixes)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "osdata_codeName",
               .majorVersion = "osdata_majorVersion",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "osdata_platform",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillOnce(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));
    EXPECT_CALL(*spRemediationDataCacheMock, addRemediationData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(SYNCHRONIZATION_STATE_HOTFIXES_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::HotfixInsert);

    EXPECT_STREQ(
        scanContext->agentId().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(
        scanContext->agentIp().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_name()));
    EXPECT_STREQ(scanContext->agentVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_version()));
    EXPECT_STREQ(scanContext->osHostName().data(), osData.hostName.c_str());
    EXPECT_STREQ(scanContext->osArchitecture().data(), osData.architecture.c_str());
    EXPECT_STREQ(scanContext->osName().data(), osData.name.c_str());
    EXPECT_STREQ(scanContext->osVersion().data(), osData.version.c_str());
    EXPECT_STREQ(scanContext->osCodeName().data(), osData.codeName.c_str());
    EXPECT_STREQ(scanContext->osMajorVersion().data(), osData.majorVersion.c_str());
    EXPECT_STREQ(scanContext->osMinorVersion().data(), osData.minorVersion.c_str());
    EXPECT_STREQ(scanContext->osPatch().data(), osData.patch.c_str());
    EXPECT_STREQ(scanContext->osBuild().data(), osData.build.c_str());
    EXPECT_STREQ(scanContext->osPlatform().data(), osData.platform.c_str());
    EXPECT_STREQ(scanContext->osKernelSysName().data(), osData.sysName.c_str());
    EXPECT_STREQ(scanContext->osKernelRelease().data(), osData.kernelRelease.c_str());
    EXPECT_STREQ(scanContext->osKernelVersion().data(), osData.kernelVersion.c_str());
    EXPECT_STREQ(scanContext->osRelease().data(), osData.release.c_str());
    EXPECT_STREQ(scanContext->osDisplayVersion().data(), osData.displayVersion.c_str());
}

TEST_F(ScanContextTest, TestSyscollectorSynchronizationIntegrityClear)
{
    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(SYNCHRONIZATION_INTEGRITY_CLEAR_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::IntegrityClear);

    EXPECT_STREQ(
        scanContext->agentId().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_id()));
    EXPECT_STREQ(
        scanContext->agentIp().data(),
        fbStringGetHelper(
            SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))->agent_info()->agent_ip()));
    EXPECT_STREQ(scanContext->agentName().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_name()));
    EXPECT_STREQ(scanContext->agentVersion().data(),
                 fbStringGetHelper(SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer))
                                       ->agent_info()
                                       ->agent_version()));
}

TEST_F(ScanContextTest, TestBuildCPEWindows10)
{
    const auto stateInfoMessage =
        R"(
            {
              "agent_info": {
                "agent_id": "001",
                "agent_ip": "any",
                "agent_name": "DESKTOP-5RL9J34"
              },
              "data": {
                "attributes": {
                  "architecture": "x86_64",
                  "checksum": "1704898816288594500",
                  "hostname": "DESKTOP-5RL9J34",
                  "os_build": "19045.3930",
                  "os_display_version": "22H2",
                  "os_major": "10",
                  "os_minor": "0",
                  "os_name": "Microsoft Windows 10 Pro",
                  "os_platform": "windows",
                  "os_release": "2009",
                  "os_version": "10.0.19045.3930",
                  "scan_time": "2024/01/10 15:00:17"
                },
                "attributes_type": "syscollector_osinfo",
                "index": "Microsoft Windows 10 Pro",
                "timestamp": ""
              },
              "data_type": "state"
            }
        )";

    const std::string osCpeRules {
        R"#(
            {
                "Microsoft Windows 10": "microsoft:windows_10_$(DISPLAY_VERSION):$(VERSION):::::"
            }
        )#"};

    const nlohmann::json osCpeMap = nlohmann::json::parse(osCpeRules);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, osCpeMaps()).WillOnce(testing::Return(osCpeMap));

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(stateInfoMessage));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>> scanContext;
    EXPECT_NO_THROW((scanContext = std::make_shared<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>>(msg)));

    EXPECT_STREQ(scanContext->osCPEName().data(), "cpe:/o:microsoft:windows_10_22h2:10.0.19045.3930:::::");
}

TEST_F(ScanContextTest, TestBuildCPECentos)
{
    const auto stateInfoMessage =
        R"(
            {
              "agent_info": {
                "agent_id": "007",
                "agent_ip": "any",
                "agent_name": "b256669deac2"
              },
              "data": {
                "attributes": {
                  "architecture": "x86_64",
                  "checksum": "1704898015502529823",
                  "hostname": "b256669deac2",
                  "os_codename": "Core",
                  "os_major": "7",
                  "os_minor": "9",
                  "os_name": "CentOS Linux",
                  "os_patch": "2009",
                  "os_platform": "centos",
                  "os_version": "7.9.2009",
                  "release": "6.2.0-39-generic",
                  "scan_time": "2024/01/10 14:46:56",
                  "sysname": "Linux",
                  "version": "#40~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 16 10:53:04 UTC 2"
                },
                "attributes_type": "syscollector_osinfo",
                "index": "CentOS Linux",
                "timestamp": ""
              },
              "data_type": "state"
            }
        )";

    const std::string osCpeRules {
        R"#(
            {
                "centos": "redhat:enterprise_linux:$(MAJOR_VERSION)"
            }
        )#"};

    const nlohmann::json osCpeMap = nlohmann::json::parse(osCpeRules);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, osCpeMaps()).WillOnce(testing::Return(osCpeMap));

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(stateInfoMessage));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>> scanContext;
    EXPECT_NO_THROW((scanContext = std::make_shared<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>>(msg)));

    EXPECT_STREQ(scanContext->osCPEName().data(), "cpe:/o:redhat:enterprise_linux:7");
}

TEST_F(ScanContextTest, TestBuildCPERedHat)
{
    const auto stateInfoMessage =
        R"(
            {
              "agent_info": {
                "agent_id": "007",
                "agent_ip": "any",
                "agent_name": "b256669deac2"
              },
              "data": {
                "attributes": {
                  "architecture": "x86_64",
                  "checksum": "1704898015502529823",
                  "hostname": "b256669deac2",
                  "os_codename": "Core",
                  "os_major": "7",
                  "os_minor": "9",
                  "os_name": "Red Hat Enterprise Linux Server",
                  "os_patch": "2009",
                  "os_platform": "rhel",
                  "os_version": "7.9.2009",
                  "release": "6.2.0-39-generic",
                  "scan_time": "2024/01/10 14:46:56",
                  "sysname": "Linux",
                  "version": "#40~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 16 10:53:04 UTC 2"
                },
                "attributes_type": "syscollector_osinfo",
                "index": "Red Hat Enterprise Linux Server",
                "timestamp": ""
              },
              "data_type": "state"
            }
        )";

    const std::string osCpeRules {
        R"#(
            {
                "rhel": "redhat:enterprise_linux:$(MAJOR_VERSION)"
            }
        )#"};

    const nlohmann::json osCpeMap = nlohmann::json::parse(osCpeRules);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, osCpeMaps()).WillOnce(testing::Return(osCpeMap));

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(stateInfoMessage));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>> scanContext;
    EXPECT_NO_THROW((scanContext = std::make_shared<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>>(msg)));

    EXPECT_STREQ(scanContext->osCPEName().data(), "cpe:/o:redhat:enterprise_linux:7");
}

TEST_F(ScanContextTest, TestBuildCPEOpensuseTumbleweed)
{
    const auto stateInfoMessage =
        R"(
            {
              "agent_info": {
                "agent_id": "018",
                "agent_ip": "any",
                "agent_name": "f8d5df70094a"
              },
              "data": {
                "attributes": {
                  "architecture": "x86_64",
                  "checksum": "1704898017508755732",
                  "hostname": "f8d5df70094a",
                  "os_name": "openSUSE Tumbleweed",
                  "os_platform": "opensuse-tumbleweed",
                  "release": "6.2.0-39-generic",
                  "scan_time": "2024/01/10 14:46:58",
                  "sysname": "Linux",
                  "version": "#40~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 16 10:53:04 UTC 2"
                },
                "attributes_type": "syscollector_osinfo",
                "index": "openSUSE Tumbleweed",
                "timestamp": ""
              },
              "data_type": "state"
            }
        )";

    const std::string osCpeRules {
        R"#(
            {
                "opensuse-tumbleweed": "opensuse:tumbleweed"
            }
        )#"};

    const nlohmann::json osCpeMap = nlohmann::json::parse(osCpeRules);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, osCpeMaps()).WillOnce(testing::Return(osCpeMap));

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(stateInfoMessage));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>> scanContext;
    EXPECT_NO_THROW((scanContext = std::make_shared<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>>(msg)));

    EXPECT_STREQ(scanContext->osCPEName().data(), "cpe:/o:opensuse:tumbleweed");
}

TEST_F(ScanContextTest, TestBuildCPEOpensuseLeap)
{
    const auto stateInfoMessage =
        R"(
            {
              "agent_info": {
                "agent_id": "018",
                "agent_ip": "any",
                "agent_name": "f8d5df70094a"
              },
              "data": {
                "attributes": {
                  "architecture": "x86_64",
                  "checksum": "1704898017508755732",
                  "hostname": "f8d5df70094a",
                  "os_name": "openSUSE Leap",
                  "os_platform": "opensuse-leap",
                  "release": "6.2.0-39-generic",
                  "scan_time": "2024/01/10 14:46:58",
                  "sysname": "Linux",
                  "version": "#40~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 16 10:53:04 UTC 2"
                },
                "attributes_type": "syscollector_osinfo",
                "index": "openSUSE Leap",
                "timestamp": ""
              },
              "data_type": "state"
            }
        )";

    const std::string osCpeRules {
        R"#(
            {
                "opensuse-leap": "opensuse:leap"
            }
        )#"};

    const nlohmann::json osCpeMap = nlohmann::json::parse(osCpeRules);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, osCpeMaps()).WillOnce(testing::Return(osCpeMap));

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(stateInfoMessage));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>> scanContext;
    EXPECT_NO_THROW((scanContext = std::make_shared<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>>(msg)));

    EXPECT_STREQ(scanContext->osCPEName().data(), "cpe:/o:opensuse:leap");
}

TEST_F(ScanContextTest, TestBuildCPENameFedora)
{
    const auto stateInfoMessage =
        R"(
            {
              "agent_info": {
                "agent_id": "019",
                "agent_ip": "any",
                "agent_name": "5624f3047059"
              },
              "data": {
                "attributes": {
                  "architecture": "x86_64",
                  "checksum": "1704898015653860088",
                  "hostname": "5624f3047059",
                  "os_codename": "\"\"",
                  "os_major": "37",
                  "os_name": "Fedora Linux",
                  "os_platform": "fedora",
                  "os_version": "37 Container Image",
                  "release": "6.2.0-39-generic",
                  "scan_time": "2024/01/10 14:46:56",
                  "sysname": "Linux",
                  "version": "#40~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 16 10:53:04 UTC 2"
                },
                "attributes_type": "syscollector_osinfo",
                "index": "Fedora Linux",
                "timestamp": ""
              },
              "data_type": "state"
            })";

    const std::string osCpeRules {
        R"#(
            {
                "fedora": "fedoraproject:fedora:$(MAJOR_VERSION)"
            }
        )#"};

    const nlohmann::json osCpeMap = nlohmann::json::parse(osCpeRules);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, osCpeMaps()).WillOnce(testing::Return(osCpeMap));

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(stateInfoMessage));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));

    std::shared_ptr<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>> scanContext;
    EXPECT_NO_THROW((scanContext = std::make_shared<TScanContext<TrampolineOsDataCache, TrampolineGlobalData>>(msg)));

    EXPECT_STREQ(scanContext->osCPEName().data(), "cpe:/o:fedoraproject:fedora:37");
}

TEST_F(ScanContextTest, TestJSONMessagePackageDelete)
{
    const auto message = nlohmann::json::parse(
        R"({
            "agent_info": {
                "agent_id" : "001",
                "agent_ip" : "10.0.0.1",
                "agent_name" : "agent1",
                "agent_version" : "v4.8.0"
            },
            "action": "deletePackage",
            "data": {
                "name" : "packageName",
                "version" : "1.0.0",
                "location" : "/usr/bin/packageName",
                "architecture": "x86_64",
                "format": "rpm",
                "item_id" : "2509fc973210e58bec81a497a2e0cdb8681b7d77"
            }
        })");

    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = &message;
    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::PackageDelete);
    EXPECT_EQ(scanContext->messageType(), MessageType::DataJSON);
    EXPECT_EQ(scanContext->agentId(), "001");
    EXPECT_EQ(scanContext->agentIp(), "10.0.0.1");
    EXPECT_EQ(scanContext->agentName(), "agent1");
    EXPECT_EQ(scanContext->agentVersion(), "v4.8.0");
    EXPECT_EQ(scanContext->packageName(), "packageName");
    EXPECT_EQ(scanContext->packageVersion(), "1.0.0");
    EXPECT_EQ(scanContext->packageLocation(), "/usr/bin/packageName");
    EXPECT_EQ(scanContext->packageArchitecture(), "x86_64");
    EXPECT_EQ(scanContext->packageFormat(), "rpm");
    EXPECT_EQ(scanContext->packageItemId(), "2509fc973210e58bec81a497a2e0cdb8681b7d77");
    EXPECT_EQ(scanContext->packageVendor(), "");
    EXPECT_EQ(scanContext->packageInstallTime(), "");
    EXPECT_EQ(scanContext->packageGroups(), "");
    EXPECT_EQ(scanContext->packageDescription(), "");
    EXPECT_EQ(scanContext->packageSize(), 0);
    EXPECT_EQ(scanContext->packagePriority(), "");
    EXPECT_EQ(scanContext->packageMultiarch(), "");
    EXPECT_EQ(scanContext->packageSource(), "");
}

TEST_F(ScanContextTest, TestJSONMessageHotfixDelete)
{
    const auto message = nlohmann::json::parse(
        R"({
            "agent_info": {
                "agent_id" : "001",
                "agent_ip" : "10.0.0.1",
                "agent_name" : "agent1",
                "agent_version" : "v4.8.0"
            },
            "action": "deleteHotfix",
            "data": {
                "hotfix" : "KB1234"
            }
        })");

    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        msg = &message;
    std::shared_ptr<scanContext_t> scanContext;
    EXPECT_NO_THROW(scanContext = std::make_shared<scanContext_t>(msg));

    EXPECT_EQ(scanContext->getType(), ScannerType::HotfixDelete);
    EXPECT_EQ(scanContext->messageType(), MessageType::DataJSON);
    EXPECT_EQ(scanContext->agentId(), "001");
    EXPECT_EQ(scanContext->agentIp(), "10.0.0.1");
    EXPECT_EQ(scanContext->agentName(), "agent1");
    EXPECT_EQ(scanContext->agentVersion(), "v4.8.0");
    EXPECT_EQ(scanContext->packageName(), "");
    EXPECT_EQ(scanContext->packageVersion(), "");
    EXPECT_EQ(scanContext->packageLocation(), "");
    EXPECT_EQ(scanContext->packageArchitecture(), "");
    EXPECT_EQ(scanContext->packageFormat(), "");
    EXPECT_EQ(scanContext->packageItemId(), "");
    EXPECT_EQ(scanContext->packageVendor(), "");
    EXPECT_EQ(scanContext->packageInstallTime(), "");
    EXPECT_EQ(scanContext->packageGroups(), "");
    EXPECT_EQ(scanContext->packageDescription(), "");
    EXPECT_EQ(scanContext->packageSize(), 0);
    EXPECT_EQ(scanContext->packagePriority(), "");
    EXPECT_EQ(scanContext->packageMultiarch(), "");
    EXPECT_EQ(scanContext->packageSource(), "");
}
