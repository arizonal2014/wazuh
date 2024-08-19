/*
 * Wazuh Vulnerability Scanner - Unit Tests
 * Copyright (C) 2015, Wazuh Inc.
 * January 2, 2024.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_deltas_generated.h"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_deltas_schema.h"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_synchronization_generated.h"
#include "../../../../shared_modules/utils/flatbuffers/include/syscollector_synchronization_schema.h"
#include "../scanOrchestrator/eventDetailsBuilder.hpp"
#include "MockDatabaseFeedManager.hpp"
#include "MockOsDataCache.hpp"
#include "TrampolineOsDataCache.hpp"
#include "TrampolineRemediationDataCache.hpp"
#include "eventDetailsBuilder_test.hpp"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "json.hpp"
#include "timeHelper.h"
#include <unistd.h>

using ::testing::_;
using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

namespace NSEventDetailsBuilderTest
{
constexpr auto TEST_DESCRIPTION_DATABASE_PATH {"queue/vd/descriptions"};

const std::string DELTA_PACKAGES_INSERTED_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "001",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal",
                    "agent_version": "4.7.1"
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
                    "version": "5.1.9-1",
                    "install_time": "1577890801",
                    "location":" "
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
                    "agent_version": "4.7.1",
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
                    "version": "5.1.9-1",
                    "install_time": "1577890801",
                    "location":" "
                },
                "operation": "DELETED"
            }
        )";

const std::string OS_SCAN_MSG =
    R"({
                "agent_info": {
                    "agent_id": "002",
                    "agent_ip": "192.168.33.30",
                    "agent_version": "4.7.1",
                    "agent_name": "Microsoft-10"
                },
                "data_type": "state",
                "data": {
                    "attributes_type": "syscollector_osinfo",
                    "attributes": {
                        "architecture": "x86_64",
                        "checksum": "1691178971959743855",
                        "hostname":"fd9b83c25f30",
                        "os_major":"10",
                        "os_minor":"0",
                        "os_build":"19045.4046",
                        "os_name":"Microsoft Windows 10 Pro",
                        "os_display_version":"22H2",
                        "os_platform":"windows",
                        "os_version":"10.0.19045.4043",
                        "scan_time":"2023/08/04 19:56:11"
                    }
                }
            })";
const std::string DELTA_PACKAGES_INSERTED_MANAGER_MSG =
    R"(
            {
                "agent_info": {
                    "agent_id": "000",
                    "agent_ip": "192.168.33.20",
                    "agent_name": "focal",
                    "agent_version": "4.7.1"
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
                    "version": "5.1.9-1",
                    "install_time": "1577890801",
                    "location":" "
                },
                "operation": "INSERTED"
            }
        )";

const std::string CVEID {"CVE-2024-1234"};
} // namespace NSEventDetailsBuilderTest

using namespace NSEventDetailsBuilderTest;

void EventDetailsBuilderTest::SetUp()
{
    std::filesystem::create_directories("queue/vd");

    // Policy manager initialization.
    const auto& configJson {nlohmann::json::parse(R"({
    "vulnerability-detection": {
        "enabled": "yes",
        "index-status": "yes",
        "cti-url": "cti-url.com"
    },
    "osdataLRUSize":1000,
    "clusterName":"cluster01",
    "clusterEnabled":true,
    "clusterNodeName":"node01"
    })")};
    PolicyManager::instance().initialize(configJson);
}

void EventDetailsBuilderTest::TearDown()
{
    spOsDataCacheMock.reset();
    spRemediationDataCacheMock.reset();
    PolicyManager::instance().teardown();
    std::filesystem::remove_all("queue/vd");
}

TEST_F(EventDetailsBuilderTest, TestSuccessfulPackageInsertedCVSS2)
{
    flatbuffers::FlatBufferBuilder fbBuilder;
    auto vulnerabilityDescriptionData =
        NSVulnerabilityScanner::CreateVulnerabilityDescriptionDirect(fbBuilder,
                                                                     "accessComplexity_test_string",
                                                                     "assignerShortName_test_string",
                                                                     "attackVector_test_string",
                                                                     "authentication_test_string",
                                                                     "availabilityImpact_test_string",
                                                                     "classification_test_string",
                                                                     "confidentialityImpact_test_string",
                                                                     "cweId_test_string",
                                                                     "datePublished_test_string",
                                                                     "dateUpdated_test_string",
                                                                     "description_test_string",
                                                                     "integrityImpact_test_string",
                                                                     "privilegesRequired_test_string",
                                                                     "reference_test_string",
                                                                     "scope_test_string",
                                                                     8.3,
                                                                     "2",
                                                                     "severity_test_string",
                                                                     "userInteraction_test_string");
    fbBuilder.Finish(vulnerabilityDescriptionData);

    auto dbWrapper = std::make_unique<Utils::RocksDBWrapper>(TEST_DESCRIPTION_DATABASE_PATH);
    rocksdb::Slice dbValue(reinterpret_cast<const char*>(fbBuilder.GetBufferPointer()), fbBuilder.GetSize());
    dbWrapper->put(CVEID, dbValue);

    auto mockGetVulnerabiltyDescriptiveInformation =
        [&](const std::string_view cveId,
            FlatbufferDataPair<NSVulnerabilityScanner::VulnerabilityDescription>& resultContainer)
    {
        dbWrapper->get(std::string(cveId), resultContainer.slice);
        resultContainer.data = const_cast<NSVulnerabilityScanner::VulnerabilityDescription*>(
            NSVulnerabilityScanner::GetVulnerabilityDescription(resultContainer.slice.data()));
    };

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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabiltyDescriptiveInformation(_, _))
        .WillRepeatedly(testing::Invoke(mockGetVulnerabiltyDescriptiveInformation));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContext =
        std::make_shared<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>(
            syscollectorDelta);
    // Mock one vulnerability
    scanContext->m_elements[CVEID] =
        R"({"operation":"INSERTED", "id":"001_ec465b7eb5fa011a336e95614072e4c7f1a65a53_CVE-2024-1234"})"_json;

    TEventDetailsBuilder<MockDatabaseFeedManager,
                         TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>
        eventDetailsBuilder(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(eventDetailsBuilder.handleRequest(scanContext));

    EXPECT_EQ(scanContext->m_elements.size(), 1);
    EXPECT_NE(scanContext->m_elements.find(CVEID), scanContext->m_elements.end());

    auto& element = scanContext->m_elements[CVEID];

    EXPECT_STREQ(element.at("operation").get_ref<const std::string&>().c_str(), "INSERTED");
    std::string elementId =
        std::string(scanContext->agentId()) + "_" + std::string(scanContext->packageItemId()) + "_" + CVEID;
    EXPECT_STREQ(element.at("id").get_ref<const std::string&>().c_str(), elementId.c_str());

    auto& elementData = scanContext->m_elements[CVEID].at("data");

    EXPECT_STREQ(elementData.at("agent").at("id").get_ref<const std::string&>().c_str(), scanContext->agentId().data());
    EXPECT_STREQ(elementData.at("agent").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->agentName().data());
    EXPECT_STREQ(elementData.at("agent").at("type").get_ref<const std::string&>().c_str(), "wazuh");
    EXPECT_STREQ(elementData.at("agent").at("version").get_ref<const std::string&>().c_str(),
                 scanContext->agentVersion().data());

    EXPECT_STREQ(elementData.at("package").at("architecture").get_ref<const std::string&>().c_str(),
                 scanContext->packageArchitecture().data());
    EXPECT_STREQ(elementData.at("package").at("description").get_ref<const std::string&>().c_str(),
                 scanContext->packageDescription().data());
    EXPECT_STREQ(elementData.at("package").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->packageName().data());
    EXPECT_THAT([elementData]() { elementData.at("package").at("path"); },
                ThrowsMessage<nlohmann::json_abi_v3_11_2::detail::out_of_range>(HasSubstr("key 'path' not found")));
    EXPECT_EQ(elementData.at("package").at("size").get_ref<const uint64_t&>(), scanContext->packageSize());
    EXPECT_STREQ(elementData.at("package").at("type").get_ref<const std::string&>().c_str(),
                 scanContext->packageFormat().data());
    EXPECT_STREQ(elementData.at("package").at("version").get_ref<const std::string&>().c_str(),
                 scanContext->packageVersion().data());

    std::string elementOsFullName;
    elementOsFullName.append(scanContext->osName().data());
    elementOsFullName.append(" ");
    elementOsFullName.append(scanContext->osPlatform().compare("darwin") == 0 ? scanContext->osCodeName().data()
                                                                              : scanContext->osVersion().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("full").get_ref<const std::string&>().c_str(),
                 elementOsFullName.c_str());
    EXPECT_STREQ(elementData.at("host").at("os").at("kernel").get_ref<const std::string&>().c_str(),
                 scanContext->osKernelRelease().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->osName().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("platform").get_ref<const std::string&>().c_str(),
                 Utils::toLowerCase(scanContext->osPlatform().data()).c_str());
    EXPECT_STREQ(elementData.at("host").at("os").at("type").get_ref<const std::string&>().c_str(),
                 Utils::toLowerCase(scanContext->osPlatform().compare("darwin") == 0 ? "macos"
                                                                                     : scanContext->osPlatform().data())
                     .c_str());
    std::string elementOsVersion = scanContext->osMajorVersion().data();
    if (!scanContext->osMinorVersion().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osMinorVersion();
    }
    if (!scanContext->osPatch().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osPatch();
    }
    if (!scanContext->osBuild().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osBuild();
    }
    EXPECT_STREQ(elementData.at("host").at("os").at("version").get_ref<const std::string&>().c_str(),
                 elementOsVersion.c_str());

    EXPECT_STREQ(elementData.at("vulnerability").at("category").get_ref<const std::string&>().c_str(), "Packages");
    EXPECT_STREQ(elementData.at("vulnerability").at("classification").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->classification()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("description").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->description()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("enumeration").get_ref<const std::string&>().c_str(), "CVE");
    EXPECT_STREQ(elementData.at("vulnerability").at("id").get_ref<const std::string&>().c_str(), CVEID.c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("reference").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->reference()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("scanner").at("vendor").get_ref<const std::string&>().c_str(),
                 "Wazuh");
    EXPECT_DOUBLE_EQ(
        elementData.at("vulnerability").at("score").at("base").get_ref<const double&>(),
        Utils::floatToDoubleRound(GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->scoreBase(), 2));
    EXPECT_STREQ(elementData.at("vulnerability").at("score").at("version").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->scoreVersion()->c_str());
    EXPECT_STREQ(
        elementData.at("vulnerability").at("severity").get_ref<const std::string&>().c_str(),
        Utils::toSentenceCase(GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->severity()->str()).c_str());
    auto clusterName = PolicyManager::instance().getClusterName();
    EXPECT_STREQ(elementData.at("wazuh").at("cluster").at("name").get_ref<const std::string&>().c_str(),
                 clusterName.c_str());
    EXPECT_STREQ(elementData.at("wazuh").at("schema").at("version").get_ref<const std::string&>().c_str(),
                 WAZUH_SCHEMA_VERSION);
    EXPECT_STREQ(elementData.at("vulnerability").at("published_at").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->datePublished()->c_str());
    EXPECT_TRUE(elementData.at("vulnerability").at("detected_at").get_ref<const std::string&>()
                <= Utils::getCurrentISO8601());
}

TEST_F(EventDetailsBuilderTest, TestSuccessfulPackageInsertedCVSS3)
{
    flatbuffers::FlatBufferBuilder fbBuilder;
    auto vulnerabilityDescriptionData =
        NSVulnerabilityScanner::CreateVulnerabilityDescriptionDirect(fbBuilder,
                                                                     "accessComplexity_test_string",
                                                                     "assignerShortName_test_string",
                                                                     "attackVector_test_string",
                                                                     "authentication_test_string",
                                                                     "availabilityImpact_test_string",
                                                                     "classification_test_string",
                                                                     "confidentialityImpact_test_string",
                                                                     "cweId_test_string",
                                                                     "datePublished_test_string",
                                                                     "dateUpdated_test_string",
                                                                     "description_test_string",
                                                                     "integrityImpact_test_string",
                                                                     "privilegesRequired_test_string",
                                                                     "reference_test_string",
                                                                     "scope_test_string",
                                                                     8.3,
                                                                     "3",
                                                                     "severity_test_string",
                                                                     "userInteraction_test_string");
    fbBuilder.Finish(vulnerabilityDescriptionData);

    auto dbWrapper = std::make_unique<Utils::RocksDBWrapper>(TEST_DESCRIPTION_DATABASE_PATH);
    rocksdb::Slice dbValue(reinterpret_cast<const char*>(fbBuilder.GetBufferPointer()), fbBuilder.GetSize());
    dbWrapper->put(CVEID, dbValue);

    auto mockGetVulnerabiltyDescriptiveInformation =
        [&](const std::string_view cveId,
            FlatbufferDataPair<NSVulnerabilityScanner::VulnerabilityDescription>& resultContainer)
    {
        dbWrapper->get(std::string(cveId), resultContainer.slice);
        resultContainer.data = const_cast<NSVulnerabilityScanner::VulnerabilityDescription*>(
            NSVulnerabilityScanner::GetVulnerabilityDescription(resultContainer.slice.data()));
    };

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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabiltyDescriptiveInformation(_, _))
        .WillRepeatedly(testing::Invoke(mockGetVulnerabiltyDescriptiveInformation));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_INSERTED_MANAGER_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContext =
        std::make_shared<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>(
            syscollectorDelta);
    scanContext->m_elements[CVEID] =
        R"({"operation":"INSERTED", "id":"000_ec465b7eb5fa011a336e95614072e4c7f1a65a53_CVE-2024-1234"})"_json;
    scanContext->m_alerts[CVEID] = nlohmann::json::object(); // Mock one alert

    TEventDetailsBuilder<MockDatabaseFeedManager,
                         TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>
        eventDetailsBuilder(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(eventDetailsBuilder.handleRequest(scanContext));

    EXPECT_EQ(scanContext->m_elements.size(), 1);
    EXPECT_NE(scanContext->m_elements.find(CVEID), scanContext->m_elements.end());

    auto& element = scanContext->m_elements[CVEID];

    EXPECT_STREQ(element.at("operation").get_ref<const std::string&>().c_str(), "INSERTED");
    std::string elementId =
        std::string(scanContext->agentId()) + "_" + std::string(scanContext->packageItemId()) + "_" + CVEID;
    EXPECT_STREQ(element.at("id").get_ref<const std::string&>().c_str(), elementId.c_str());

    auto& elementData = scanContext->m_elements[CVEID].at("data");

    EXPECT_STREQ(elementData.at("agent").at("ephemeral_id").get_ref<const std::string&>().c_str(),
                 scanContext->clusterNodeName().data());
    EXPECT_STREQ(elementData.at("agent").at("id").get_ref<const std::string&>().c_str(), scanContext->agentId().data());
    EXPECT_STREQ(elementData.at("agent").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->agentName().data());
    EXPECT_STREQ(elementData.at("agent").at("type").get_ref<const std::string&>().c_str(), "wazuh");
    EXPECT_STREQ(elementData.at("agent").at("version").get_ref<const std::string&>().c_str(),
                 scanContext->agentVersion().data());

    EXPECT_STREQ(elementData.at("package").at("architecture").get_ref<const std::string&>().c_str(),
                 scanContext->packageArchitecture().data());
    EXPECT_STREQ(elementData.at("package").at("description").get_ref<const std::string&>().c_str(),
                 scanContext->packageDescription().data());
    EXPECT_STREQ(elementData.at("package").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->packageName().data());
    EXPECT_THAT([elementData]() { elementData.at("package").at("path"); },
                ThrowsMessage<nlohmann::json_abi_v3_11_2::detail::out_of_range>(HasSubstr("key 'path' not found")));
    EXPECT_EQ(elementData.at("package").at("size").get_ref<const uint64_t&>(), scanContext->packageSize());
    EXPECT_STREQ(elementData.at("package").at("type").get_ref<const std::string&>().c_str(),
                 scanContext->packageFormat().data());
    EXPECT_STREQ(elementData.at("package").at("version").get_ref<const std::string&>().c_str(),
                 scanContext->packageVersion().data());

    std::string elementOsFullName;
    elementOsFullName.append(scanContext->osName().data());
    elementOsFullName.append(" ");
    elementOsFullName.append(scanContext->osPlatform().compare("darwin") == 0 ? scanContext->osCodeName().data()
                                                                              : scanContext->osVersion().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("full").get_ref<const std::string&>().c_str(),
                 elementOsFullName.c_str());
    EXPECT_STREQ(elementData.at("host").at("os").at("kernel").get_ref<const std::string&>().c_str(),
                 scanContext->osKernelRelease().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->osName().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("platform").get_ref<const std::string&>().c_str(),
                 Utils::toLowerCase(scanContext->osPlatform().data()).c_str());
    EXPECT_STREQ(elementData.at("host").at("os").at("type").get_ref<const std::string&>().c_str(),
                 Utils::toLowerCase(scanContext->osPlatform().compare("darwin") == 0 ? "macos"
                                                                                     : scanContext->osPlatform().data())
                     .c_str());
    std::string elementOsVersion = scanContext->osMajorVersion().data();
    if (!scanContext->osMinorVersion().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osMinorVersion();
    }
    if (!scanContext->osPatch().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osPatch();
    }
    if (!scanContext->osBuild().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osBuild();
    }
    EXPECT_STREQ(elementData.at("host").at("os").at("version").get_ref<const std::string&>().c_str(),
                 elementOsVersion.c_str());

    EXPECT_STREQ(elementData.at("vulnerability").at("category").get_ref<const std::string&>().c_str(), "Packages");
    EXPECT_STREQ(elementData.at("vulnerability").at("classification").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->classification()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("description").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->description()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("enumeration").get_ref<const std::string&>().c_str(), "CVE");
    EXPECT_STREQ(elementData.at("vulnerability").at("id").get_ref<const std::string&>().c_str(), CVEID.c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("reference").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->reference()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("scanner").at("vendor").get_ref<const std::string&>().c_str(),
                 "Wazuh");
    EXPECT_DOUBLE_EQ(
        elementData.at("vulnerability").at("score").at("base").get_ref<const double&>(),
        Utils::floatToDoubleRound(GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->scoreBase(), 2));
    EXPECT_STREQ(elementData.at("vulnerability").at("score").at("version").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->scoreVersion()->c_str());
    EXPECT_STREQ(
        elementData.at("vulnerability").at("severity").get_ref<const std::string&>().c_str(),
        Utils::toSentenceCase(GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->severity()->str()).c_str());
    auto clusterName = PolicyManager::instance().getClusterName();
    EXPECT_STREQ(elementData.at("wazuh").at("cluster").at("name").get_ref<const std::string&>().c_str(),
                 clusterName.c_str());
    EXPECT_STREQ(elementData.at("wazuh").at("schema").at("version").get_ref<const std::string&>().c_str(),
                 WAZUH_SCHEMA_VERSION);
    EXPECT_STREQ(elementData.at("vulnerability").at("published_at").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->datePublished()->c_str());
    EXPECT_TRUE(elementData.at("vulnerability").at("detected_at").get_ref<const std::string&>()
                <= Utils::getCurrentISO8601());
}

TEST_F(EventDetailsBuilderTest, TestSuccessfulPackageDeleted)
{
    flatbuffers::FlatBufferBuilder fbBuilder;
    auto vulnerabilityDescriptionData =
        NSVulnerabilityScanner::CreateVulnerabilityDescriptionDirect(fbBuilder,
                                                                     "accessComplexity_test_string",
                                                                     "assignerShortName_test_string",
                                                                     "attackVector_test_string",
                                                                     "authentication_test_string",
                                                                     "availabilityImpact_test_string",
                                                                     "classification_test_string",
                                                                     "confidentialityImpact_test_string",
                                                                     "cweId_test_string",
                                                                     "datePublished_test_string",
                                                                     "dateUpdated_test_string",
                                                                     "description_test_string",
                                                                     "integrityImpact_test_string",
                                                                     "privilegesRequired_test_string",
                                                                     "reference_test_string",
                                                                     "scope_test_string",
                                                                     8.3,
                                                                     "3",
                                                                     "severity_test_string",
                                                                     "userInteraction_test_string");
    fbBuilder.Finish(vulnerabilityDescriptionData);

    auto dbWrapper = std::make_unique<Utils::RocksDBWrapper>(TEST_DESCRIPTION_DATABASE_PATH);
    rocksdb::Slice dbValue(reinterpret_cast<const char*>(fbBuilder.GetBufferPointer()), fbBuilder.GetSize());
    dbWrapper->put(CVEID, dbValue);

    auto mockGetVulnerabiltyDescriptiveInformation =
        [&](const std::string_view cveId,
            FlatbufferDataPair<NSVulnerabilityScanner::VulnerabilityDescription>& resultContainer)
    {
        dbWrapper->get(std::string(cveId), resultContainer.slice);
        resultContainer.data = const_cast<NSVulnerabilityScanner::VulnerabilityDescription*>(
            NSVulnerabilityScanner::GetVulnerabilityDescription(resultContainer.slice.data()));
    };

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
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabiltyDescriptiveInformation(_, _))
        .WillRepeatedly(testing::Invoke(mockGetVulnerabiltyDescriptiveInformation));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_deltas_SCHEMA));
    ASSERT_TRUE(parser.Parse(DELTA_PACKAGES_DELETED_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorDeltas::GetDelta(reinterpret_cast<const char*>(buffer));
    auto scanContext =
        std::make_shared<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>(
            syscollectorDelta);
    scanContext->m_elements[CVEID] = R"({"operation":"INSERTED"})"_json;

    TEventDetailsBuilder<MockDatabaseFeedManager,
                         TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>
        eventDetailsBuilder(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(eventDetailsBuilder.handleRequest(scanContext));

    EXPECT_EQ(scanContext->m_elements.size(), 1);
    EXPECT_NE(scanContext->m_elements.find(CVEID), scanContext->m_elements.end());
}

TEST_F(EventDetailsBuilderTest, TestSuccessfulOsInserted)
{
    flatbuffers::FlatBufferBuilder fbBuilder;
    auto vulnerabilityDescriptionData =
        NSVulnerabilityScanner::CreateVulnerabilityDescriptionDirect(fbBuilder,
                                                                     "accessComplexity_test_string",
                                                                     "assignerShortName_test_string",
                                                                     "attackVector_test_string",
                                                                     "authentication_test_string",
                                                                     "availabilityImpact_test_string",
                                                                     "classification_test_string",
                                                                     "confidentialityImpact_test_string",
                                                                     "cweId_test_string",
                                                                     "datePublished_test_string",
                                                                     "dateUpdated_test_string",
                                                                     "description_test_string",
                                                                     "integrityImpact_test_string",
                                                                     "privilegesRequired_test_string",
                                                                     "reference_test_string",
                                                                     "scope_test_string",
                                                                     8.3,
                                                                     "2",
                                                                     "severity_test_string",
                                                                     "userInteraction_test_string");
    fbBuilder.Finish(vulnerabilityDescriptionData);

    auto dbWrapper = std::make_unique<Utils::RocksDBWrapper>(TEST_DESCRIPTION_DATABASE_PATH);
    rocksdb::Slice dbValue(reinterpret_cast<const char*>(fbBuilder.GetBufferPointer()), fbBuilder.GetSize());
    dbWrapper->put(CVEID, dbValue);

    auto mockGetVulnerabiltyDescriptiveInformation =
        [&](const std::string_view cveId,
            FlatbufferDataPair<NSVulnerabilityScanner::VulnerabilityDescription>& resultContainer)
    {
        dbWrapper->get(std::string(cveId), resultContainer.slice);
        resultContainer.data = const_cast<NSVulnerabilityScanner::VulnerabilityDescription*>(
            NSVulnerabilityScanner::GetVulnerabilityDescription(resultContainer.slice.data()));
    };

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
    EXPECT_CALL(*spOsDataCacheMock, setOsData(_, _)).Times(1);
    EXPECT_CALL(*spOsDataCacheMock, getOsData(_)).WillRepeatedly(testing::Return(osData));

    spRemediationDataCacheMock = std::make_shared<MockRemediationDataCache>();
    EXPECT_CALL(*spRemediationDataCacheMock, getRemediationData(_)).WillRepeatedly(testing::Return(Remediation {}));

    auto spDatabaseFeedManagerMock = std::make_shared<MockDatabaseFeedManager>();
    EXPECT_CALL(*spDatabaseFeedManagerMock, getVulnerabiltyDescriptiveInformation(_, _))
        .WillRepeatedly(testing::Invoke(mockGetVulnerabiltyDescriptiveInformation));

    flatbuffers::Parser parser;
    ASSERT_TRUE(parser.Parse(syscollector_synchronization_SCHEMA));
    ASSERT_TRUE(parser.Parse(OS_SCAN_MSG.c_str()));
    uint8_t* buffer = parser.builder_.GetBufferPointer();
    std::variant<const SyscollectorDeltas::Delta*, const SyscollectorSynchronization::SyncMsg*, const nlohmann::json*>
        syscollectorDelta = SyscollectorSynchronization::GetSyncMsg(reinterpret_cast<const char*>(buffer));
    auto scanContext =
        std::make_shared<TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>(
            syscollectorDelta);
    scanContext->m_elements[CVEID] =
        R"({"operation":"INSERTED", "id":"002_Microsoft Windows 10 Pro_CVE-2024-1234"})"_json;

    TEventDetailsBuilder<MockDatabaseFeedManager,
                         TScanContext<TrampolineOsDataCache, GlobalData, TrampolineRemediationDataCache>>
        eventDetailsBuilder(spDatabaseFeedManagerMock);

    EXPECT_NO_THROW(eventDetailsBuilder.handleRequest(scanContext));

    EXPECT_EQ(scanContext->m_elements.size(), 1);
    EXPECT_NE(scanContext->m_elements.find(CVEID), scanContext->m_elements.end());

    auto& element = scanContext->m_elements[CVEID];

    EXPECT_STREQ(element.at("operation").get_ref<const std::string&>().c_str(), "INSERTED");
    std::string elementId =
        std::string(scanContext->agentId()) + "_" + std::string(scanContext->osName()) + "_" + CVEID;
    EXPECT_STREQ(element.at("id").get_ref<const std::string&>().c_str(), elementId.c_str());

    auto& elementData = scanContext->m_elements[CVEID].at("data");

    EXPECT_STREQ(elementData.at("agent").at("id").get_ref<const std::string&>().c_str(), scanContext->agentId().data());
    EXPECT_STREQ(elementData.at("agent").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->agentName().data());
    EXPECT_STREQ(elementData.at("agent").at("type").get_ref<const std::string&>().c_str(), "wazuh");
    EXPECT_STREQ(elementData.at("agent").at("version").get_ref<const std::string&>().c_str(),
                 scanContext->agentVersion().data());

    EXPECT_STREQ(elementData.at("wazuh").at("schema").at("version").get_ref<const std::string&>().c_str(),
                 WAZUH_SCHEMA_VERSION);

    std::string elementOsFullName;
    elementOsFullName.append(scanContext->osName().data());
    elementOsFullName.append(" ");
    elementOsFullName.append(scanContext->osPlatform().compare("darwin") == 0 ? scanContext->osCodeName().data()
                                                                              : scanContext->osVersion().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("full").get_ref<const std::string&>().c_str(),
                 elementOsFullName.c_str());
    EXPECT_STREQ(elementData.at("host").at("os").at("name").get_ref<const std::string&>().c_str(),
                 scanContext->osName().data());
    EXPECT_STREQ(elementData.at("host").at("os").at("platform").get_ref<const std::string&>().c_str(),
                 Utils::toLowerCase(scanContext->osPlatform().data()).c_str());
    EXPECT_STREQ(elementData.at("host").at("os").at("type").get_ref<const std::string&>().c_str(),
                 Utils::toLowerCase(scanContext->osPlatform().compare("darwin") == 0 ? "macos"
                                                                                     : scanContext->osPlatform().data())
                     .c_str());
    std::string elementOsVersion = scanContext->osMajorVersion().data();
    if (!scanContext->osMinorVersion().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osMinorVersion();
    }
    if (!scanContext->osPatch().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osPatch();
    }
    if (!scanContext->osBuild().empty())
    {
        elementOsVersion += ".";
        elementOsVersion += scanContext->osBuild();
    }
    EXPECT_STREQ(elementData.at("host").at("os").at("version").get_ref<const std::string&>().c_str(),
                 elementOsVersion.c_str());

    EXPECT_STREQ(elementData.at("vulnerability").at("category").get_ref<const std::string&>().c_str(), "OS");
    EXPECT_STREQ(elementData.at("vulnerability").at("classification").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->classification()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("description").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->description()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("enumeration").get_ref<const std::string&>().c_str(), "CVE");
    EXPECT_STREQ(elementData.at("vulnerability").at("id").get_ref<const std::string&>().c_str(), CVEID.c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("reference").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->reference()->c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("scanner").at("vendor").get_ref<const std::string&>().c_str(),
                 "Wazuh");
    EXPECT_DOUBLE_EQ(
        elementData.at("vulnerability").at("score").at("base").get_ref<const double&>(),
        Utils::floatToDoubleRound(GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->scoreBase(), 2));
    EXPECT_STREQ(elementData.at("vulnerability").at("score").at("version").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->scoreVersion()->c_str());
    EXPECT_STREQ(
        elementData.at("vulnerability").at("severity").get_ref<const std::string&>().c_str(),
        Utils::toSentenceCase(GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->severity()->str()).c_str());
    auto clusterName = PolicyManager::instance().getClusterName();
    EXPECT_STREQ(elementData.at("wazuh").at("cluster").at("name").get_ref<const std::string&>().c_str(),
                 clusterName.c_str());
    EXPECT_STREQ(elementData.at("vulnerability").at("published_at").get_ref<const std::string&>().c_str(),
                 GetVulnerabilityDescription(fbBuilder.GetBufferPointer())->datePublished()->c_str());
    EXPECT_TRUE(elementData.at("vulnerability").at("detected_at").get_ref<const std::string&>()
                <= Utils::getCurrentISO8601());
}
