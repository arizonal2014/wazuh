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

#include "factoryOrchestrator_test.hpp"
#include "MockDatabaseFeedManager.hpp"
#include "MockIndexerConnector.hpp"

class FactoryOrchestratorTest : public ::testing::Test
{
protected:
    // LCOV_EXCL_START
    FactoryOrchestratorTest() = default;
    ~FactoryOrchestratorTest() override = default;
    // LCOV_EXCL_STOP

    /**
     * @brief Set up for every test.
     *
     */
    void SetUp() override { m_inventoryDatabase = std::make_unique<Utils::RocksDBWrapper>(INVENTORY_DB_PATH); }

    /**
     * @brief Tear down for every test.
     *
     */
    void TearDown() override { std::remove(INVENTORY_DB_PATH); }

    /**
     * @brief RocksDB inventory database.
     *
     */
    std::unique_ptr<Utils::RocksDBWrapper> m_inventoryDatabase;
};

enum class ScannerMockID : int
{
    PACKAGE_SCANNER = 0,
    EVENT_PACKAGE_ALERT_DETAILS_BUILDER = 1,
    SCAN_OS_ALERT_DETAILS_BUILDER = 2,
    CVE_SOLVED_ALERT_DETAILS_BUILDER = 3,
    EVENT_DETAILS_BUILDER = 4,
    ALERT_CLEAR_BUILDER = 5,
    OS_SCANNER = 6,
    CLEAN_ALL_AGENT_INVENTORY = 7,
    EVENT_DELETE_INVENTORY = 8,
    EVENT_INSERT_INVENTORY = 9,
    SCAN_INVENTORY_SYNC = 10,
    CVE_SOLVED_INVENTORY_SYNC = 11,
    CLEAR_SEND_REPORT = 12,
    EVENT_SEND_REPORT = 13,
    RESULT_INDEXER = 14,
    BUILD_ALL_AGENT_LIST_CONTEXT = 15,
    BUILD_SINGLE_AGENT_LIST_CONTEXT = 16,
    CLEAN_SINGLE_AGENT_INVENTORY = 17,
    SCAN_AGENT_LIST = 18,
    GLOBAL_INVENTORY_SYNC = 19,
    HOTFIX_INSERT = 20,
    ARRAY_RESULT_INDEXER = 21
};

/**
 * @brief Generic fake base class
 */
template<ScannerMockID param = ScannerMockID::PACKAGE_SCANNER>
class TFakeClass : public AbstractHandler<std::shared_ptr<std::vector<ScannerMockID>>>
{
public:
    ScannerMockID m_id {param}; ///< Identifier.

    /**
     * @brief Construct a new TFakeClass object.
     *
     * @param databaseFeedManager MockDatabaseFeedManager instance.
     */
    TFakeClass(std::shared_ptr<MockDatabaseFeedManager>& databaseFeedManager) {};
    /**
     * @brief Construct a new TFakeClass object.
     *
     * @param inventoryDatabase RocksDBWrapper instance for inventory storage.
     * @param subOrchestration AbstractHandler instance.
     */
    TFakeClass(Utils::RocksDBWrapper& inventoryDatabase,
               std::shared_ptr<AbstractHandler<std::shared_ptr<std::vector<ScannerMockID>>>> subOrchestration) {};
    /**
     * @brief Construct a new TFakeClass object.
     *
     * @param inventoryDatabase RocksDBWrapper instance for inventory storage.
     */
    TFakeClass(Utils::RocksDBWrapper& inventoryDatabase) {};
    /**
     * @brief Construct a new TFakeClass object.
     *
     * @param reportDispatcher Report dispatcher instance.
     */
    TFakeClass(std::shared_ptr<ReportDispatcher> reportDispatcher) {};
    /**
     * @brief Construct a new TFakeClass object.
     *
     * @param indexerConnector TIndexerConnector instance.
     */
    TFakeClass(std::shared_ptr<MockIndexerConnector> indexerConnector) {};
    /**
     * @brief Construct a new TFakeClass object.
     *
     */
    TFakeClass(std::shared_ptr<AbstractHandler<std::shared_ptr<std::vector<ScannerMockID>>>>,
               std::shared_ptr<AbstractHandler<std::shared_ptr<std::vector<ScannerMockID>>>>) {};
    /**
     * @brief Construct a new TFakeClass object.
     */
    TFakeClass() = default;
    virtual ~TFakeClass() = default;

    /**
     * @brief Handles request and passes control to the next step of the chain.
     *
     * @param data Mocked context data
     * @return std::shared_ptr<std::vector<std::string>>
     */
    std::shared_ptr<std::vector<ScannerMockID>> handleRequest(std::shared_ptr<std::vector<ScannerMockID>> data) override
    {
        data->push_back(m_id);
        return AbstractHandler<std::shared_ptr<std::vector<ScannerMockID>>>::handleRequest(std::move(data));
    }
};

/*
 * @brief Test the chain creation for packages.
 */
TEST_F(FactoryOrchestratorTest, TestScannerTypePackageInsert)
{
    // Create the orchestrator for PackageInsert.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::PackageInsert,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 6);
    EXPECT_EQ(context->at(0), ScannerMockID::PACKAGE_SCANNER);
    EXPECT_EQ(context->at(1), ScannerMockID::EVENT_INSERT_INVENTORY);
    EXPECT_EQ(context->at(2), ScannerMockID::EVENT_DETAILS_BUILDER);
    EXPECT_EQ(context->at(3), ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER);
    EXPECT_EQ(context->at(4), ScannerMockID::EVENT_SEND_REPORT);
    EXPECT_EQ(context->at(5), ScannerMockID::RESULT_INDEXER);
}

/*
 * @brief Test the chain deletion for packages.
 */
TEST_F(FactoryOrchestratorTest, TestScannerTypePackageDelete)
{
    // Create the orchestrator for PackageDelete.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::PackageDelete,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 4);
    EXPECT_EQ(context->at(0), ScannerMockID::EVENT_DELETE_INVENTORY);
    EXPECT_EQ(context->at(1), ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER);
    EXPECT_EQ(context->at(2), ScannerMockID::EVENT_SEND_REPORT);
    EXPECT_EQ(context->at(3), ScannerMockID::RESULT_INDEXER);
}

/*
 * @brief Test the chain creation for packages.
 */
TEST_F(FactoryOrchestratorTest, TestScannerTypeIntegrityClear)
{
    // Create the orchestrator for IntegrityClear.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::IntegrityClear,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 3);
    EXPECT_EQ(context->at(0), ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY);
    EXPECT_EQ(context->at(1), ScannerMockID::ALERT_CLEAR_BUILDER);
    EXPECT_EQ(context->at(2), ScannerMockID::CLEAR_SEND_REPORT);
}

/*
 * @brief Test the chain creation for os.
 */
TEST_F(FactoryOrchestratorTest, TestScannerTypeOs)
{
    // Create the orchestrator for Os.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::Os,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 6);
    EXPECT_EQ(context->at(0), ScannerMockID::OS_SCANNER);
    EXPECT_EQ(context->at(1), ScannerMockID::SCAN_INVENTORY_SYNC);
    EXPECT_EQ(context->at(2), ScannerMockID::EVENT_DETAILS_BUILDER);
    EXPECT_EQ(context->at(3), ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER);
    EXPECT_EQ(context->at(4), ScannerMockID::EVENT_SEND_REPORT);
    EXPECT_EQ(context->at(5), ScannerMockID::RESULT_INDEXER);
}

/**
 * @brief Test the chain creation for CleanupAllData.
 */
TEST_F(FactoryOrchestratorTest, TestCreationCleanUpAllData)
{
    // Create the orchestrator for CleanUpAllDAta.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::CleanupAllAgentData,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 1);
    EXPECT_EQ(context->at(0), ScannerMockID::CLEAN_ALL_AGENT_INVENTORY);
}

/**
 * @brief Test the chain creation for ReScanAllAgents.
 */
TEST_F(FactoryOrchestratorTest, TestCreationReScanAllAgents)
{
    // Create the orchestrator for ReScanAllAgents.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::ReScanAllAgents,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 3);
    EXPECT_EQ(context->at(0), ScannerMockID::CLEAN_ALL_AGENT_INVENTORY);
    EXPECT_EQ(context->at(1), ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT);
    EXPECT_EQ(context->at(2), ScannerMockID::SCAN_AGENT_LIST);
}

/**
 * @brief Test the chain creation for ReScanSingleAgent.
 */
TEST_F(FactoryOrchestratorTest, TestCreationReScanSingleAgent)
{
    // Create the orchestrator for ReScanSingleAgent.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::ReScanSingleAgent,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 3);
    EXPECT_EQ(context->at(0), ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY);
    EXPECT_EQ(context->at(1), ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT);
    EXPECT_EQ(context->at(2), ScannerMockID::SCAN_AGENT_LIST);
}

TEST_F(FactoryOrchestratorTest, TestCreationCleanUpAgentData)
{
    // Create the orchestrator for CleanupSingleAgentData.
    auto orchestration = TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                                              TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                                              TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                                              TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                                              TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                                              TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                                              TFakeClass<ScannerMockID::OS_SCANNER>,
                                              TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                                              TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                                              TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                                              TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                                              TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                                              TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                                              TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                                              TFakeClass<ScannerMockID::RESULT_INDEXER>,
                                              MockDatabaseFeedManager,
                                              MockIndexerConnector,
                                              std::vector<ScannerMockID>,
                                              TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                                              TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                                              TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                                              TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                                              TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                                              TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                                              TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::
        create(ScannerType::CleanupSingleAgentData, nullptr, nullptr, *m_inventoryDatabase, nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 1);
    EXPECT_EQ(context->at(0), ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY);
}

TEST_F(FactoryOrchestratorTest, TestCreationHotfixInsert)
{
    // Create the orchestrator for HotfixInsert.
    // Create the orchestrator for CleanupSingleAgentData.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::HotfixInsert,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 5);
    EXPECT_EQ(context->at(0), ScannerMockID::HOTFIX_INSERT);
    EXPECT_EQ(context->at(1), ScannerMockID::CVE_SOLVED_INVENTORY_SYNC);
    EXPECT_EQ(context->at(2), ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER);
    EXPECT_EQ(context->at(3), ScannerMockID::EVENT_SEND_REPORT);
    EXPECT_EQ(context->at(4), ScannerMockID::ARRAY_RESULT_INDEXER);
}

/*
 * @brief Test the creation of an invalid scanner.
 */
TEST_F(FactoryOrchestratorTest, TestCreationInvalidScannerType)
{
    // Create the orchestrator with invalid ScannerType.
    ScannerType invalidScannerType {-1};

    try
    {
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(invalidScannerType,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_STREQ(e.what(), "Invalid scanner type");
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error";
    }
}

TEST_F(FactoryOrchestratorTest, TestCreationGlobalSyncInventory)
{
    // Create the orchestrator for CleanupSingleAgentData.
    auto orchestration =
        TFactoryOrchestrator<TFakeClass<ScannerMockID::PACKAGE_SCANNER>,
                             TFakeClass<ScannerMockID::EVENT_PACKAGE_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::SCAN_OS_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_ALERT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::EVENT_DETAILS_BUILDER>,
                             TFakeClass<ScannerMockID::ALERT_CLEAR_BUILDER>,
                             TFakeClass<ScannerMockID::OS_SCANNER>,
                             TFakeClass<ScannerMockID::CLEAN_ALL_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_DELETE_INVENTORY>,
                             TFakeClass<ScannerMockID::EVENT_INSERT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CVE_SOLVED_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::CLEAR_SEND_REPORT>,
                             TFakeClass<ScannerMockID::EVENT_SEND_REPORT>,
                             TFakeClass<ScannerMockID::RESULT_INDEXER>,
                             MockDatabaseFeedManager,
                             MockIndexerConnector,
                             std::vector<ScannerMockID>,
                             TFakeClass<ScannerMockID::BUILD_ALL_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::BUILD_SINGLE_AGENT_LIST_CONTEXT>,
                             TFakeClass<ScannerMockID::CLEAN_SINGLE_AGENT_INVENTORY>,
                             TFakeClass<ScannerMockID::SCAN_AGENT_LIST>,
                             TFakeClass<ScannerMockID::GLOBAL_INVENTORY_SYNC>,
                             TFakeClass<ScannerMockID::HOTFIX_INSERT>,
                             TFakeClass<ScannerMockID::ARRAY_RESULT_INDEXER>>::create(ScannerType::GlobalSyncInventory,
                                                                                      nullptr,
                                                                                      nullptr,
                                                                                      *m_inventoryDatabase,
                                                                                      nullptr);

    auto context = std::make_shared<std::vector<ScannerMockID>>();

    EXPECT_NO_THROW(orchestration->handleRequest(context));
    EXPECT_EQ(context->size(), 1);
    EXPECT_EQ(context->at(0), ScannerMockID::GLOBAL_INVENTORY_SYNC);
}
