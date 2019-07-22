/*
 * 2018.5.14
 * (totoro) kem2182@yonsei.ac.kr
 * ADDB Test Commands for custom STL implementations
 */

#include "server.h"
#include "sds.h"
#include "stl.h"
#include "proto/stl.pb-c.h"

#include <assert.h>
#include <string.h>

/* --*-- Caution --*--
 * Uses these commands for testing only...
 */

/* Vector Add / Get / Remove Interface Test command. */

/*
 * testVectorInterfaceCommand
 * Tests Vector interface(Add / Get / Remove).
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTVECTORINTERFACE
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testVectorInterfaceCommand(client *c) {
    {
        // Vector Type [ANY] (DEFAULT)
        Vector v;
        vectorInit(&v);
        serverLog(LL_DEBUG, "[ADDB_TEST][VECTOR][ANY] Test ANY type Vector");
        assert(v.size == 0);
        assert(v.count == 0);
        const char *values[] = {
            "TEST_VECTOR_ANY_VALUE_1", "TEST_VECTOR_ANY_VALUE_2",
            "TEST_VECTOR_ANY_VALUE_3",
        };

        // Add test
        vectorAdd(&v, (void *) values[0]);
        vectorAdd(&v, (void *) values[1]);
        vectorAdd(&v, (void *) values[2]);
        assert(v.count == 3);
        for (size_t i = 0; i < vectorCount(&v); ++i) {
            assert(strcmp(values[i], vectorGet(&v, i)) == 0);
        }
        // Pop test
        void *popedDatum;
        popedDatum = vectorPop(&v);
        assert(v.count == 2 && (strcmp(popedDatum, values[2]) == 0));
        // Unlink test
        void *unlinkedDatum;
        unlinkedDatum = vectorUnlink(&v, 0);
        assert(v.count == 1 && (strcmp(unlinkedDatum, values[0]) == 0));
        unlinkedDatum = vectorUnlink(&v, 0);
        assert(v.count == 0 && (strcmp(unlinkedDatum, values[1]) == 0));
        vectorFreeDeep(&v);
    }
    {
        // Vector Type [LONG]
        Vector v;
        vectorTypeInit(&v, STL_TYPE_LONG);
        serverLog(LL_DEBUG, "[ADDB_TEST][VECTOR][LONG] Test LONG type Vector");
        assert(v.size == 0);
        assert(v.count == 0);
        const long values[] = { 1, 2, 3 };
        vectorAdd(&v, (void *) values[0]);
        vectorAdd(&v, (void *) values[1]);
        vectorAdd(&v, (void *) values[2]);
        assert(v.count == 3);
        for (size_t i = 0; i < vectorCount(&v); ++i) {
            assert(values[i] == (long) vectorGet(&v, i));
        }
        // Pop test
        long popedDatum;
        popedDatum = (long) vectorPop(&v);
        assert(v.count == 2 && popedDatum == values[2]);
        vectorDelete(&v, 0);
        assert(v.count == 1);
        vectorDelete(&v, 0);
        assert(v.count == 0);
        vectorFreeDeep(&v);
    }
    {
        // Vector Type [SDS]
        Vector v;
        vectorTypeInit(&v, STL_TYPE_SDS);
        serverLog(LL_DEBUG, "[ADDB_TEST][VECTOR][SDS] Test SDS type Vector");
        assert(v.size == 0);
        assert(v.count == 0);
        const sds values[] = {
            sdsnew("TEST_VECTOR_SDS_VALUE_1"),
            sdsnew("TEST_VECTOR_SDS_VALUE_2"),
            sdsnew("TEST_VECTOR_SDS_VALUE_3"),
        };
        vectorAdd(&v, (void *) values[0]);
        vectorAdd(&v, (void *) values[1]);
        vectorAdd(&v, (void *) values[2]);
        assert(v.count == 3);
        for (size_t i = 0; i < vectorCount(&v); ++i) {
            assert(sdscmp(values[i], (sds) vectorGet(&v, i)) == 0);
        }
        // Pop test
        sds popedDatum;
        popedDatum = (sds) vectorPop(&v);
        assert(v.count == 2 && sdscmp(popedDatum, values[2]) == 0);
        vectorDelete(&v, 0);
        assert(v.count == 1);
        vectorDelete(&v, 0);
        assert(v.count == 0);
        vectorFreeDeep(&v);
        sdsfree(popedDatum);
    }
    {
        // Vector Type [ROBJ]
        Vector v;
        vectorTypeInit(&v, STL_TYPE_ROBJ);
        serverLog(LL_DEBUG, "[ADDB_TEST][VECTOR][ROBJ] Test ROBJ type Vector");
        assert(v.size == 0);
        assert(v.count == 0);
        const robj *values[] = {
            createStringObject("TEST_VECTOR_ROBJ_VALUE_1",
                               strlen("TEST_VECTOR_ROBJ_VALUE_1")),
            createStringObject("TEST_VECTOR_ROBJ_VALUE_2",
                               strlen("TEST_VECTOR_ROBJ_VALUE_2")),
            createStringObject("TEST_VECTOR_ROBJ_VALUE_3",
                               strlen("TEST_VECTOR_ROBJ_VALUE_3")),
        };
        vectorAdd(&v, (void *) values[0]);
        vectorAdd(&v, (void *) values[1]);
        vectorAdd(&v, (void *) values[2]);
        assert(v.count == 3);
        for (size_t i = 0; i < vectorCount(&v); ++i) {
            assert(sdscmp(values[i]->ptr, ((robj *) vectorGet(&v, i))->ptr) == 0);
        }
        robj *popedDatum;
        popedDatum = (robj *) vectorPop(&v);
        assert(sdscmp(popedDatum->ptr, values[2]->ptr) == 0);
        vectorDelete(&v, 0);
        assert(v.count == 1);
        vectorDelete(&v, 0);
        assert(v.count == 0);
        vectorFreeDeep(&v);
        decrRefCount(popedDatum);
    }

    addReply(c, shared.ok);
}

/* Stack Push / Pop / Free Interface Test command. */

/*
 * testStackInterfaceCommand
 * Tests Stack interface (Push / Pop / Free).
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTSTACKINTERFACE
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testStackInterfaceCommand(client *c) {
    {
        // Stack Type [ANY] (DEFAULT)
        Stack s;
        stackInit(&s);
        serverLog(LL_DEBUG, "[ADDB_TEST][STACK][ANY] Test ANY type Stack");
        assert(s.data.size == 0);
        assert(s.data.count == 0);
        const char *values[] = {
            "TEST_STACK_ANY_VALUE_1", "TEST_STACK_ANY_VALUE_2",
            "TEST_STACK_ANY_VALUE_3",
        };
        // Push test
        stackPush(&s, (void *) values[0]);
        stackPush(&s, (void *) values[1]);
        stackPush(&s, (void *) values[2]);
        assert(s.data.count == 3);
        // Pop test
        void *popedDatum;
        popedDatum = stackPop(&s);
        assert(s.data.count == 2 && (strcmp(popedDatum, values[2]) == 0));
        popedDatum = stackPop(&s);
        assert(s.data.count == 1 && (strcmp(popedDatum, values[1]) == 0));
        popedDatum = stackPop(&s);
        assert(s.data.count == 0 && (strcmp(popedDatum, values[0]) == 0));
        stackFreeDeep(&s);
    }
    {
        // Stack Type [LONG]
        Stack s;
        stackTypeInit(&s, STL_TYPE_LONG);
        serverLog(LL_DEBUG, "[ADDB_TEST][STACK][LONG] Test LONG type Vector");
        assert(s.data.size == 0);
        assert(s.data.count == 0);
        const long values[] = { 1, 2, 3 };
        stackPush(&s, (void *) values[0]);
        stackPush(&s, (void *) values[1]);
        stackPush(&s, (void *) values[2]);
        assert(s.data.count == 3);
        // Pop test
        long popedDatum;
        popedDatum = (long) stackPop(&s);
        assert(s.data.count == 2 && popedDatum == values[2]);
        popedDatum = (long) stackPop(&s);
        assert(s.data.count == 1 && popedDatum == values[1]);
        popedDatum = (long) stackPop(&s);
        assert(s.data.count == 0 && popedDatum == values[0]);
        stackFreeDeep(&s);
    }
    {
        // Stack Type [SDS]
        Stack s;
        stackTypeInit(&s, STL_TYPE_SDS);
        serverLog(LL_DEBUG, "[ADDB_TEST][STACK][SDS] Test SDS type Vector");
        assert(s.data.size == 0);
        assert(s.data.count == 0);
        const sds values[] = {
            sdsnew("TEST_STACK_SDS_VALUE_1"),
            sdsnew("TEST_STACK_SDS_VALUE_2"),
            sdsnew("TEST_STACK_SDS_VALUE_3"),
        };
        stackPush(&s, (void *) values[0]);
        stackPush(&s, (void *) values[1]);
        stackPush(&s, (void *) values[2]);
        assert(s.data.count == 3);
        // Pop test
        sds popedDatum;
        popedDatum = (sds) stackPop(&s);
        assert(s.data.count == 2 && sdscmp(popedDatum, values[2]) == 0);
        sdsfree(popedDatum);
        popedDatum = (sds) stackPop(&s);
        assert(s.data.count == 1 && sdscmp(popedDatum, values[1]) == 0);
        sdsfree(popedDatum);
        popedDatum = (sds) stackPop(&s);
        assert(s.data.count == 0 && sdscmp(popedDatum, values[0]) == 0);
        sdsfree(popedDatum);
        stackFreeDeep(&s);
    }
    {
        // Stack Type [ROBJ]
        Stack s;
        stackTypeInit(&s, STL_TYPE_ROBJ);
        serverLog(LL_DEBUG, "[ADDB_TEST][STACK][ROBJ] Test ROBJ type Vector");
        assert(s.data.size == 0);
        assert(s.data.count == 0);
        const robj *values[] = {
            createStringObject("TEST_STACK_ROBJ_VALUE_1",
                               strlen("TEST_STACK_ROBJ_VALUE_1")),
            createStringObject("TEST_STACK_ROBJ_VALUE_2",
                               strlen("TEST_STACK_ROBJ_VALUE_2")),
            createStringObject("TEST_STACK_ROBJ_VALUE_3",
                               strlen("TEST_STACK_ROBJ_VALUE_3")),
        };
        stackPush(&s, (void *) values[0]);
        stackPush(&s, (void *) values[1]);
        stackPush(&s, (void *) values[2]);
        assert(s.data.count == 3);
        // Pop test
        robj *popedDatum;
        popedDatum = (robj *) stackPop(&s);
        assert(s.data.count == 2 &&
                sdscmp(popedDatum->ptr, values[2]->ptr) == 0);
        decrRefCount(popedDatum);
        popedDatum = (robj *) stackPop(&s);
        assert(s.data.count == 1 &&
                sdscmp(popedDatum->ptr, values[1]->ptr) == 0);
        decrRefCount(popedDatum);
        popedDatum = (robj *) stackPop(&s);
        assert(s.data.count == 0 &&
                sdscmp(popedDatum->ptr, values[0]->ptr) == 0);
        decrRefCount(popedDatum);
        stackFreeDeep(&s);
    }

    addReply(c, shared.ok);
}

/*
 * testProtobufCommand
 * Tests Protobuf-Vector interface (Push / Pop / Free).
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTSTACKINTERFACE
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testProtoVectorCommand(client *c) {
    {
        // Test storing string data to ProtoVector
        serverLog(LL_DEBUG,
                  "[ADDB_TEST][ProtoVector] Test String type ProtoVector");

        // Source - Naive Vector Interface
        Vector source;
        vectorTypeInit(&source, STL_TYPE_SDS);
        // ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
        for (size_t i = 0; i < 10; ++i) {
            sds entry = sdscatfmt(sdsempty(), "%U", i);
            vectorAdd(&source, entry);
        }

        // Target - Proto Vector Interface
        ProtoVector target = PROTO_VECTOR__INIT;
        target.type = STL_TYPE__STRING;
        target.n_values = 10;
        target.values = zmalloc(sizeof(StlEntry *) * target.n_values);

        // ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
        for (size_t i = 0; i < 10; ++i) {
            sds raw_entry = sdscatfmt(sdsempty(), "%U", i);
            target.values[i] = (StlEntry *) zmalloc(sizeof(StlEntry));
            stl_entry__init(target.values[i]);
            target.values[i]->value_case = STL_ENTRY__VALUE__STRING;
            target.values[i]->_string = raw_entry;
        }

        // Compare
        for (size_t i = 0; i < 10; ++i) {
            sds source_entry = (sds) vectorGet(&source, i);
            char *target_entry = (char *) target.values[i]->_string;
            assert(strcmp(source_entry, target_entry) == 0);
        }

        vectorFreeDeep(&source);
        for (size_t i = 0; i < target.n_values; ++i) {
            zfree(target.values[i]->_string);
            zfree(target.values[i]);
        }
        zfree(target.values);
        serverLog(LL_DEBUG, "[ADDB_TEST][ProtoVector] SUCCESS");
    }
    {
        // Test storing long data to ProtoVector
        serverLog(LL_DEBUG,
                  "[ADDB_TEST][ProtoVector] Test Integer type ProtoVector");

        // Source - Naive Vector Interface
        Vector source;
        vectorTypeInit(&source, STL_TYPE_LONG);
        // [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        for (size_t i = 0; i < 10; ++i) {
            vectorAdd(&source, (void *) i);
        }

        // Target - Proto Vector Interface
        ProtoVector target = PROTO_VECTOR__INIT;
        target.type = STL_TYPE__LONG;
        target.n_values = 10;
        target.values = zmalloc(sizeof(StlEntry *) * target.n_values);

        // [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        for (size_t i = 0; i < 10; ++i) {
            target.values[i] = (StlEntry *) zmalloc(sizeof(StlEntry));
            target.values[i]->value_case = STL_ENTRY__VALUE__LONG;
            target.values[i]->_long = i;
        }

        // Compare
        for (size_t i = 0; i < 10; ++i) {
            long source_entry = (long) vectorGet(&source, i);
            long target_entry = (long) target.values[i]->_long;
            assert(source_entry == target_entry);
        }

        vectorFree(&source);
        for (size_t i = 0; i < target.n_values; ++i) {
            zfree(target.values[i]);
        }
        zfree(target.values);
        serverLog(LL_DEBUG, "[ADDB_TEST][ProtoVector] SUCCESS");
    }
    {
        // Test ProtoVector Serialization-Deserialization
        serverLog(
            LL_DEBUG,
            "[ADDB_TEST][ProtoVector] Test ProtoVector Serialization-Deserialization");

        // Source - Proto Vector Interface with String
        ProtoVector source = PROTO_VECTOR__INIT;
        source.type = STL_TYPE__STRING;
        source.n_values = 10;
        source.values = zmalloc(sizeof(StlEntry *) * source.n_values);

        // ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
        for (size_t i = 0; i < 10; ++i) {
            sds raw_entry = sdscatfmt(sdsempty(), "%U", i);
            source.values[i] = (StlEntry *) zmalloc(sizeof(StlEntry));
            stl_entry__init(source.values[i]);
            source.values[i]->value_case = STL_ENTRY__VALUE__STRING;
            source.values[i]->_string = raw_entry;
        }

        // Protobuf Serialization
        size_t serialized_len = proto_vector__get_packed_size(&source);
        char *serialized = zcalloc(sizeof(char) * serialized_len);
        proto_vector__pack(&source, (uint8_t *) serialized);

        // Protobuf Deserialization
        ProtoVector *target = proto_vector__unpack(
            NULL, serialized_len, (const uint8_t *) serialized);
        for (size_t i = 0; i < 10; ++i) {
            assert(
                source.values[i]->value_case == target->values[i]->value_case);
            assert(
                strcmp(source.values[i]->_string, target->values[i]->_string) == 0);
        }

        // Free protobuf-related data...
        for (size_t i = 0; i < source.n_values; ++i) {
            sdsfree(source.values[i]->_string);
            zfree(source.values[i]);
        }
        zfree(source.values);
        zfree(serialized);
        proto_vector__free_unpacked(target, NULL);
        serverLog(LL_DEBUG, "[ADDB_TEST][ProtoVector] SUCCESS");
    }

    addReply(c, shared.ok);
}
