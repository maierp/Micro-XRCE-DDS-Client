#include <micrortps/client/client.h>
#include <micrortps/client/xrce_protocol_spec.h>

#include <microcdr/microcdr.h>

#include <gtest/gtest.h>
#include <cstdint>
#include <unistd.h>

#include <fstream>

#define BUFFER_SIZE 4096
#define STATUS_TRIES_WAIT 100

typedef struct ShapeTopic
{
    uint32_t color_length;
    char*    color;
    uint32_t x;
    uint32_t y;
    uint32_t size;

} ShapeTopic;

bool serialize_shape_topic(MicroBuffer* writer, const AbstractTopic* topic_structure);
bool deserialize_shape_topic(MicroBuffer* reader, AbstractTopic* topic_serialization);

void on_shape_topic(XRCEInfo info, const void* topic, void* args);
void on_status(XRCEInfo info, uint8_t operation, uint8_t status, void* args);

void printl_shape_topic(const ShapeTopic* shape_topic);

class ClientTests : public ::testing::Test
{
    public:
        ClientTests()
        {
            state = new_udp_client_state(BUFFER_SIZE, 2020, 2019);

            statusObjectId = 0x0000;
            statusRequestId = 0x0000;
            statusOperation = 0xFF;
            statusImplementation = 0xFF;

            last_object = 0x0000;
            last_request = 0x0000;
        }

        ~ClientTests()
        {
            free_client_state(state);
        }

        void waitStatus()
        {
            /*int statusWaitCounter = 0;
            while(!receive_from_agent(state) && statusWaitCounter < STATUS_TRIES_WAIT)
            {
                usleep(1000);
                statusWaitCounter++;
            }*/

            bool expectedTimeout = false;
            bool timeout = false;
            if(!receive_from_agent(state))
                timeout = true;

            ASSERT_EQ(timeout, expectedTimeout);
        }

        void checkStatus(uint8_t operation)
        {
            ASSERT_EQ(statusObjectId, last_object);
            ASSERT_EQ(statusRequestId, last_request);
            ASSERT_EQ(statusOperation, operation);
            ASSERT_EQ(statusImplementation, STATUS_OK);
        }

        uint16_t createClient()
        {
            XRCEInfo info = create_client(state, on_status, this);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            waitStatus();

            return info.object_id;
        }

        uint16_t createParticipant()
        {
            XRCEInfo info = create_participant(state);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            waitStatus();

            return info.object_id;
        }

        uint16_t createPublisher(uint16_t participant_id)
        {
            XRCEInfo info = create_publisher(state, participant_id);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            waitStatus();

            return info.object_id;
        }

        uint16_t createSubscriber(uint16_t participant_id)
        {
            XRCEInfo info = create_subscriber(state, participant_id);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            waitStatus();

            return info.object_id;
        }

        uint16_t createDataWriter(uint16_t participant_id, uint16_t publisher_id)
        {
            String xml;
            std::ifstream in("data_writer_profile.xml", std::ifstream::in);
            in.seekg (0, in.end);
            xml.length = in.tellg();
            in.seekg (0, in.beg);
            xml.data = new char[xml.length];
            in.read(xml.data, xml.length);

            XRCEInfo info = create_data_writer(state, participant_id, publisher_id, xml);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            delete xml.data;

            waitStatus();

            return info.object_id;
        }

        uint16_t createDataReader(uint16_t participant_id, uint16_t subscriber_id)
        {
            String xml;
            std::ifstream in("data_reader_profile.xml", std::ifstream::in);
            in.seekg (0, in.end);
            xml.length = in.tellg();
            in.seekg (0, in.beg);
            xml.data = new char[xml.length];
            in.read(xml.data, xml.length);

            XRCEInfo info = create_data_reader(state, participant_id, subscriber_id, xml);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            delete xml.data;

            waitStatus();

            return info.object_id;
        }

        void writeData(uint16_t data_writer_id)
        {
            char topicColor[64] = "PURPLE";
            uint32_t length = strlen(topicColor) + 1;
            ShapeTopic shape_topic = {length, topicColor, 100, 100, 50};
            XRCEInfo info = write_data(state, data_writer_id, serialize_shape_topic, &shape_topic);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            waitStatus();
        }

        void deleteXRCEObject(uint16_t id)
        {
            XRCEInfo info = delete_resource(state, id);
            last_object = info.object_id;
            last_request = info.request_id;
            send_to_agent(state);

            waitStatus();
        }

        ClientState* state;

        uint16_t statusObjectId;
        uint16_t statusRequestId;
        uint8_t statusOperation;
        uint8_t statusImplementation;

        uint16_t last_request;
        uint16_t last_object;
};

bool serialize_shape_topic(MicroBuffer* writer, const AbstractTopic* topic_structure)
{
    ShapeTopic* topic = (ShapeTopic*) topic_structure->topic;

    serialize_uint32_t(writer, topic->color_length);
    serialize_array_char(writer, topic->color, topic->color_length);
    serialize_uint32_t(writer, topic->x);
    serialize_uint32_t(writer, topic->y);
    serialize_uint32_t(writer, topic->size);

    return true;
}

bool deserialize_shape_topic(MicroBuffer* reader, AbstractTopic* topic_structure)
{
    ShapeTopic* topic = (ShapeTopic*)malloc(sizeof(ShapeTopic));

    deserialize_uint32_t(reader, &topic->color_length);
    topic->color = (char*)malloc(sizeof(topic->color_length));
    deserialize_array_char(reader, topic->color, topic->color_length);
    deserialize_uint32_t(reader, &topic->x);
    deserialize_uint32_t(reader, &topic->y);
    deserialize_uint32_t(reader, &topic->size);

    topic_structure->topic = topic;

    return true;
}

void on_shape_topic(XRCEInfo info, const void* vtopic, void* args)
{
    ShapeTopic* topic = (ShapeTopic*) vtopic;
    printl_shape_topic(topic);

    free(topic->color);
    free(topic);
}

void on_status(XRCEInfo info, uint8_t operation, uint8_t status, void* args)
{
    ClientTests* test = static_cast<ClientTests*>(args);

    test->statusObjectId = info.object_id;
    test->statusRequestId = info.request_id;
    test->statusOperation = operation;
    test->statusImplementation = status;
}

void printl_shape_topic(const ShapeTopic* shape_topic)
{
    printf("        %s[%s | x: %u | y: %u | size: %u]%s\n",
            "\e[1;34m",
            shape_topic->color,
            shape_topic->x,
            shape_topic->y,
            shape_topic->size,
            "\e[0m");
}

TEST_F(ClientTests, CreateDeleteClient)
{
    uint16_t client_id = createClient();
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(client_id);
    checkStatus(STATUS_LAST_OP_DELETE);
}

TEST_F(ClientTests, CreateDeleteParticipant)
{
    uint16_t client_id = createClient();
    uint16_t participant_id = createParticipant();
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(participant_id);
    checkStatus(STATUS_LAST_OP_DELETE);
    deleteXRCEObject(client_id);
}

TEST_F(ClientTests, CreateDeletePublisher)
{
    uint16_t client_id = createClient();
    uint16_t participant_id = createParticipant();
    uint16_t publisher_id = createPublisher(participant_id);
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(publisher_id);
    checkStatus(STATUS_LAST_OP_DELETE);
    deleteXRCEObject(participant_id);
    deleteXRCEObject(client_id);
}

TEST_F(ClientTests, CreateDeleteSubscriber)
{
    uint16_t client_id = createClient();
    uint16_t participant_id = createParticipant();
    uint16_t subscriber_id = createSubscriber(participant_id);
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(subscriber_id);
    checkStatus(STATUS_LAST_OP_DELETE);
    deleteXRCEObject(participant_id);
    deleteXRCEObject(client_id);
}

TEST_F(ClientTests, CreateDeleteDataWriter)
{
    uint16_t client_id = createClient();
    uint16_t participant_id = createParticipant();
    uint16_t publisher_id = createPublisher(participant_id);
    uint16_t data_writer_id = createDataWriter(participant_id, publisher_id);
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(data_writer_id);
    checkStatus(STATUS_LAST_OP_DELETE);
    deleteXRCEObject(publisher_id);
    deleteXRCEObject(participant_id);
    deleteXRCEObject(client_id);
}

TEST_F(ClientTests, CreateDeleteDataReader)
{
    uint16_t client_id = createClient();
    uint16_t participant_id = createParticipant();
    uint16_t subscriber_id = createSubscriber(participant_id);
    uint16_t data_reader_id = createDataReader(participant_id, subscriber_id);
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(data_reader_id);
    checkStatus(STATUS_LAST_OP_DELETE);
    deleteXRCEObject(subscriber_id);
    deleteXRCEObject(participant_id);
    deleteXRCEObject(client_id);
}

TEST_F(ClientTests, WriteData)
{
    uint16_t client_id = createClient();
    uint16_t participant_id = createParticipant();
    uint16_t publisher_id = createPublisher(participant_id);
    uint16_t data_writer_id = createDataWriter(participant_id, publisher_id);
    writeData(data_writer_id);
    checkStatus(STATUS_LAST_OP_CREATE);

    deleteXRCEObject(data_writer_id);
    deleteXRCEObject(publisher_id);
    deleteXRCEObject(participant_id);
    deleteXRCEObject(client_id);
}