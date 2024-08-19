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

#include "../../../../feedmanager/include/databaseFeedManager.hpp"
//#include "MockDatabaseFeedManager.hpp"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/idl.h"
#include <array>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using ::testing::_;
class PackageScannerTest : public ::testing::Test
{
protected:
    // LCOV_EXCL_START
    PackageScannerTest() = default;
    ~PackageScannerTest() override = default;

    /**
     * @brief Set the environment for testing.
     *
     */
    void SetUp() override;

    /**
     * @brief Clean the environment after testing.
     *
     */
    void TearDown() override;
    // LCOV_EXCL_STOP
};

namespace NSPackageScannerTest
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
                    "version": "5.1.9",
                    "install_time": "1577890801"
                },
                "operation": "INSERTED"
            }
        )";

const std::string DELTA_PACKAGES_INSERTED_MSG_WITHOUT_VENDOR =
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
                    "vendor": " ",
                    "version": "5.1.9",
                    "install_time": "1577890801"
                },
                "operation": "INSERTED"
            }
        )";

const std::string DELTA_PACKAGES_INSERTED_MSG_WRONG_VERSION =
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
                    "version": "2016",
                    "install_time": "1577890801"
                },
                "operation": "INSERTED"
            }
        )";

const std::string CANDIDATES_AFFECTED_LESS_THAN_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.2.0",
                                "status": "affected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_AFFECTED_LESS_THAN_INPUT_WITH_GENERIC_VENDOR =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.2.0",
                                "status": "affected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ],
                        "vendor" : "testVendor"
                    }
                ]
            }
        )";

const std::string CANDIDATES_AFFECTED_LESS_THAN_INPUT_WITH_UBUNTU_VENDOR =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.2.0",
                                "status": "affected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ],
                        "vendor" : "ubuntu developers <ubuntu-devel-discuss@lists.ubuntu.com>"
                    }
                ]
            }
        )";

const std::string CANDIDATES_AFFECTED_LESS_THAN_OR_EQUAL_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThanOrEqual": "5.2.0",
                                "status": "affected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_AFFECTED_LESS_THAN_WITH_VERSION_NOT_ZERO_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.2.0",
                                "status": "affected",
                                "version": "5.1.0",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_UNAFFECTED_LESS_THAN_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.2.0",
                                "status": "unaffected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_AFFECTED_EQUAL_TO_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "status": "affected",
                                "version": "5.1.9",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_UNAFFECTED_EQUAL_TO_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "status": "unaffected",
                                "version": "5.1.9",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_DEFAULT_STATUS_AFFECTED_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 0,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.1.0",
                                "status": "affected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::string CANDIDATES_DEFAULT_STATUS_UNAFFECTED_INPUT =
    R"(
            {
                "candidates": [
                    {
                        "cveId": "CVE-2024-1234",
                        "defaultStatus": 1,
                        "platforms": [
                            "upstream"
                        ],
                        "versions": [
                            {
                                "lessThan": "5.1.0",
                                "status": "affected",
                                "version": "0",
                                "versionType": "custom"
                            }
                        ]
                    }
                ]
            }
        )";

const std::array<const char*, 2> INCLUDE_DIRECTORIES = {FLATBUFFER_SCHEMAS_DIR, nullptr};

const std::string CANDIDATES_FLATBUFFER_SCHEMA_PATH {std::string(FLATBUFFER_SCHEMAS_DIR)
                                                     + "/vulnerabilityCandidate.fbs"};

const std::string CVEID {"CVE-2024-1234"};

const nlohmann::json CNA_MAPPINGS = R"***(
    {
      "cnaMapping": {
        "alas": "alas_$(MAJOR_VERSION)",
        "alma": "alma_$(MAJOR_VERSION)",
        "redhat": "redhat_$(MAJOR_VERSION)",
        "suse": "$(PLATFORM)_$(MAJOR_VERSION)"
      },
      "majorVersionEquivalence": {
        "amzn": {
          "2018": "1"
        }
      },
      "platformEquivalence": {
        "sled": "suse_desktop",
        "sles": "suse_server"
      }
    }
    )***"_json;

} // namespace NSPackageScannerTest

using namespace NSPackageScannerTest;

void PackageScannerTest::SetUp() {}

void PackageScannerTest::TearDown() {}

TEST_F(PackageScannerTest, TestPackageAffectedEqualTo)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        const char* includeDirectories[] = {INCLUDE_DIRECTORIES[0], INCLUDE_DIRECTORIES[1]};
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), includeDirectories)
                 && fbParser.Parse(CANDIDATES_AFFECTED_EQUAL_TO_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray = NSVulnerabilityScanner::GetScanVulnerabilityCandidateArray(
            reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };
    /*
        Os osData {.hostName = "osdata_hostname",
                   .architecture = "osdata_architecture",
                   .name = "osdata_name",
                   .codeName = "upstream",
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

        auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
        EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
        EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
            .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
        EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

        ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
        auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

        EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

        TPackageScanner<MockDatabaseFeedManager, TrampolineScanContext> packageScanner(spDatabaseFeedManagerMock);

        std::shared_ptr<TrampolineScanContext> scanContextResult;
        EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

        ASSERT_TRUE(scanContextResult != nullptr);

        EXPECT_EQ(scanContextResult->m_elements.size(), 1);
        EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

        EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
        EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

        auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
        EXPECT_EQ(matchCondition.condition, MatchRuleCondition::Equal);
        EXPECT_STREQ(matchCondition.version.c_str(), "5.1.9");
        */
}
/*
TEST_F(PackageScannerTest, TestPackageUnaffectedEqualTo)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_UNAFFECTED_EQUAL_TO_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    EXPECT_TRUE(scanContextResult == nullptr);
}

TEST_F(PackageScannerTest, TestPackageAffectedLessThan)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::LessThan);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.2.0");
}

TEST_F(PackageScannerTest, TestPackageAffectedLessThanVendorMissing)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_INPUT_WITH_UBUNTU_VENDOR.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG_WITHOUT_VENDOR.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult == nullptr);
}

TEST_F(PackageScannerTest, TestPackageAffectedLessThanVendorMismatch)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_INPUT_WITH_GENERIC_VENDOR.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult == nullptr);
}

TEST_F(PackageScannerTest, TestPackageAffectedLessThanVendorMatch)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_INPUT_WITH_UBUNTU_VENDOR.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::LessThan);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.2.0");
}

TEST_F(PackageScannerTest, TestPackageAffectedLessThanOrEqual)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_OR_EQUAL_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::LessThanOrEqual);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.2.0");
}

TEST_F(PackageScannerTest, TestPackageAffectedLessThanWithVersionNotZero)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_WITH_VERSION_NOT_ZERO_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::LessThan);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.2.0");
}

TEST_F(PackageScannerTest, TestPackageUnaffectedLessThan)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_UNAFFECTED_LESS_THAN_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    EXPECT_TRUE(scanContextResult == nullptr);
}

TEST_F(PackageScannerTest, TestPackageDefaultStatusAffected)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_DEFAULT_STATUS_AFFECTED_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::DefaultStatus);
}

TEST_F(PackageScannerTest, TestPackageDefaultStatusUnaffected)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_DEFAULT_STATUS_UNAFFECTED_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    EXPECT_TRUE(scanContextResult == nullptr);
}

TEST_F(PackageScannerTest, TestPackageGetVulnerabilitiesCandidatesGeneratesException)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        throw std::runtime_error("Invalid package/cna name.");
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    EXPECT_TRUE(scanContextResult == nullptr);
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToAlma8)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "8",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "alma",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("alma"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("alma_8", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToAlas1)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "2018",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "amzn",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("alas"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("alas_1", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToAlas2)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "2",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "amzn",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("alas"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("alas_2", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToAlas2022)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "2022",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "amzn",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("alas"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("alas_2022", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToRedHat7)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "7",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "redhat",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("redhat"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("redhat_7", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToSLED)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "15",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "sled",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("suse"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("suse_desktop_15", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestPackageAffectedEqualToSLES)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "15",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "sles",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("suse"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("suse_server_15", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestcheckAndTranslatePackage)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_EQUAL_TO_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));

    std::vector<PackageData> mockPackageData = {
        PackageData {.name = "translatedProduct", .vendor = "translatedVendor"}};
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TrampolineScanContext> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::Equal);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.1.9");
}

TEST_F(PackageScannerTest, TestVersionTranslation)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_EQUAL_TO_INPUT.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));

    std::vector<PackageData> mockPackageData = {
        PackageData {.name = "translatedProduct", .vendor = "translatedVendor", .version = "5.1.9"}};
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _)).WillOnce(testing::Return(mockPackageData));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG_WRONG_VERSION.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal =
        std::make_shared<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>(
            syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>,
                    TrampolineGlobalData>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::Equal);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.1.9");
    spGlobalDataMock.reset();
}

TEST_F(PackageScannerTest, TestVendorAndVersionTranslation)
{
    auto mockGetVulnerabilitiesCandidates =
        [&](const std::string& cnaName,
            const PackageData& package,
            const std::function<bool(const std::string& cnaName,
                                     const PackageData& package,
                                     const NSVulnerabilityScanner::ScanVulnerabilityCandidate&)>& callback)
    {
        std::string candidatesFlatbufferSchemaStr;

        // Read schemas from filesystem.
        bool valid =
            flatbuffers::LoadFile(CANDIDATES_FLATBUFFER_SCHEMA_PATH.c_str(), false, &candidatesFlatbufferSchemaStr);
        ASSERT_EQ(valid, true);

        // Parse schemas and JSON example.
        flatbuffers::Parser fbParser;
        valid = (fbParser.Parse(candidatesFlatbufferSchemaStr.c_str(), INCLUDE_DIRECTORIES)
                 && fbParser.Parse(CANDIDATES_AFFECTED_LESS_THAN_INPUT_WITH_GENERIC_VENDOR.c_str()));
        ASSERT_EQ(valid, true);

        auto candidatesArray =
            GetScanVulnerabilityCandidateArray(reinterpret_cast<const uint8_t*>(fbParser.builder_.GetBufferPointer()));

        if (candidatesArray)
        {
            for (const auto& candidate : *candidatesArray->candidates())
            {
                if (callback(cnaName, package, *candidate))
                {
                    // If the candidate is vulnerable, we stop looking for.
                    break;
                }
            }
        }
    };

    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(_, _, _))
        .WillOnce(testing::Invoke(mockGetVulnerabilitiesCandidates));

    std::vector<PackageData> mockPackageData = {
        PackageData {.name = "translatedProduct", .vendor = "testVendor", .version = "5.1.9"}};
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _)).WillOnce(testing::Return(mockPackageData));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG_WRONG_VERSION.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal =
        std::make_shared<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>(
            syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>,
                    TrampolineGlobalData>
        packageScanner(spDatabaseFeedManagerMock);

    std::shared_ptr<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>> scanContextResult;
    EXPECT_NO_THROW(scanContextResult = packageScanner.handleRequest(scanContextOriginal));

    ASSERT_TRUE(scanContextResult != nullptr);

    EXPECT_EQ(scanContextResult->m_elements.size(), 1);
    EXPECT_NE(scanContextResult->m_elements.find(CVEID), scanContextResult->m_elements.end());

    EXPECT_EQ(scanContextResult->m_matchConditions.size(), 1);
    EXPECT_NE(scanContextResult->m_matchConditions.find(CVEID), scanContextResult->m_matchConditions.end());

    auto& matchCondition = scanContextResult->m_matchConditions[CVEID];
    EXPECT_EQ(matchCondition.condition, MatchRuleCondition::LessThan);
    EXPECT_STREQ(matchCondition.version.c_str(), "5.2.0");
    spGlobalDataMock.reset();
}

TEST_F(PackageScannerTest, TestGetCnaNameByPrefix)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "15",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "sles",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameBySource(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByPrefix(_, _)).WillOnce(testing::Return("suse"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("suse_server_15", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestGetCnaNameByContains)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "15",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "sles",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameBySource(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByPrefix(_, _)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByContains(_, _)).WillOnce(testing::Return("suse"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("suse_server_15", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestGetCnaNameBySource)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "15",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "sles",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameBySource(_)).WillOnce(testing::Return("cnaName"));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByPrefix(_, _)).Times(0);
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByContains(_, _)).Times(0);
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates("cnaName", _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();
    EXPECT_CALL(*spGlobalDataMock, cnaMappings()).WillOnce(testing::Return(CNA_MAPPINGS));

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}

TEST_F(PackageScannerTest, TestGetDefaultCna)
{
    Os osData {.hostName = "osdata_hostname",
               .architecture = "osdata_architecture",
               .name = "osdata_name",
               .codeName = "upstream",
               .majorVersion = "15",
               .minorVersion = "osdata_minorVersion",
               .patch = "osdata_patch",
               .build = "osdata_build",
               .platform = "sles",
               .version = "osdata_version",
               .release = "osdata_release",
               .displayVersion = "osdata_displayVersion",
               .sysName = "osdata_sysName",
               .kernelVersion = "osdata_kernelVersion",
               .kernelRelease = "osdata_kernelRelease"};

    spOsDataCacheMock = std::make_shared<MockOsDataCache>();
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByFormat(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameBySource(_)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByPrefix(_, _)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getCnaNameByContains(_, _)).WillOnce(testing::Return(""));
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabilitiesCandidates(DEFAULT_CNA, _, _));
    EXPECT_CALL(*spDatabaseFeedManagerMock, checkAndTranslatePackage(_, _));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContextOriginal = std::make_shared<TrampolineScanContext>(syscollectorDelta);

    spGlobalDataMock = std::make_shared<MockGlobalData>();

    TPackageScanner<MockDatabaseFeedManager,
                    TrampolineScanContext,
                    TrampolineGlobalData,
                    TrampolineRemediationDataCache>
        packageScanner(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(packageScanner.handleRequest(scanContextOriginal));
}
*/