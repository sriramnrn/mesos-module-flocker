#include <stout/try.hpp>
#include "../../libisolator/flocker-isolator.hpp"
#include "mock_flocker_control_service_client.hpp"
#include <stout/os.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "test_flocker_isolator.hpp"

using ::testing::Return;

using namespace mesos::slave;
using namespace mesos;
using namespace std;

FlockerIsolatorTest::FlockerIsolatorTest() { };

FlockerIsolatorTest::~FlockerIsolatorTest() {};

void FlockerIsolatorTest::SetUp() {};

void FlockerIsolatorTest::TearDown() {};

TEST_F(FlockerIsolatorTest, IsolatorCreateSetsFlockerIpAndPort) {

    Parameters parameters;

    string flockerControlIp = "192.1.2.3";
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_ip");
    parameter->set_value(flockerControlIp);

    int flockerControlPort = 4523;
    parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_port");
    parameter->set_value(stringify(flockerControlPort));

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);
    if (result.isError()) {
        cerr << "Could not create Flocker isolator" << endl;
    }

    EXPECT_EQ(flockerControlIp, result.get()->getFlockerControlClient()->getFlockerControlIp());
    EXPECT_EQ(flockerControlPort, result.get()->getFlockerControlClient()->getFlockerControlPort());
}

TEST_F(FlockerIsolatorTest, IsolatorCreateWithoutParametersReturnsError) {
    Parameters parameters;

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);

    EXPECT_TRUE(result.isError());
}

TEST_F(FlockerIsolatorTest, IsolatorCreateWithoutIpReturnsError) {
    Parameters parameters;

    int flockerControlPort = 4523;
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_port");
    parameter->set_value(stringify(flockerControlPort));

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);

    EXPECT_TRUE(result.isError());
}

TEST_F(FlockerIsolatorTest, IsolatorCreateWithoutPortReturnsError) {
    Parameters parameters;

    string flockerControlIp = "192.1.2.3";
    Parameter* parameter = parameters.add_parameter();
    parameter->set_key("flocker_control_ip");
    parameter->set_value(flockerControlIp);

    Try<FlockerIsolator*> result = FlockerIsolator::create(parameters);

    EXPECT_TRUE(result.isError());
}

TEST_F(FlockerIsolatorTest, IsolatorPrepareCallsFlockerControlService) {

    const string ip = "192.1.2.3";
    uint16_t port = 1234;

    MockFlockerControlServiceClient flockerControlClient(ip, port);

    EXPECT_CALL(flockerControlClient, getNodeId()).WillOnce(Return(Try<string>::some("fef7fa02-c8c2-4c52-96b5-de70a8ef1925")));

    UUID uuid = UUID::fromString("fef7fa02-c8c2-4c52-96b5-de70a8ef1925");

    EXPECT_CALL(flockerControlClient, createDataSet(uuid)).WillOnce(Return(Try<string>::some(
        "{\"deleted\": false, \"dataset_id\": \"e66d949c-ae91-4446-9115-824722a1e4b0\", \"primary\": \"fef7fa02-c8c2-4c52-96b5-de70a8ef1925\", \"metadata\": {}}"
    )));

    FlockerIsolator *isolator = new FlockerIsolator(&flockerControlClient);

    ContainerID containerId;
    containerId.set_value("befa2b13da05");

    ExecutorInfo executor;
    executor.mutable_executor_id()->set_value("default");
    executor.mutable_command()->set_value("/bin/sleep");
    executor.mutable_command()->add_arguments("60");
    executor.set_name("Test Executor (/bin/sleep)");

    Result<string> user = os::user();

    isolator->prepare(containerId, executor, "/tmp", user.get());

    EXPECT_EQ(1, 1);
}

