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
 * testProtoVectorCommand
 * Tests Protobuf-Vector basic logics.
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTPROTOVECTOR
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testProtoVectorCommand(client *c) {
    {
        // Test storing sds data to ProtoVector
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
        target.type = STL_TYPE__SDS;
        target.n_values = 10;
        target.values = zmalloc(sizeof(StlEntry *) * target.n_values);

        // ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
        for (size_t i = 0; i < 10; ++i) {
            sds raw_entry = sdscatfmt(sdsempty(), "%U", i);
            target.values[i] = (StlEntry *) zmalloc(sizeof(StlEntry));
            stl_entry__init(target.values[i]);
            target.values[i]->value_case = STL_ENTRY__VALUE__SDS;
            target.values[i]->_sds = sds2proto(raw_entry);
        }

        // Compare
        for (size_t i = 0; i < 10; ++i) {
            sds source_entry = (sds) vectorGet(&source, i);
            sds target_entry = proto2sds(target.values[i]->_sds);
            assert(sdscmp(source_entry, target_entry) == 0);
        }

        vectorFreeDeep(&source);
        for (size_t i = 0; i < target.n_values; ++i) {
            sds target_entry_sds = proto2sds(target.values[i]->_sds);
            sdsfree(target_entry_sds);
            zfree(target.values[i]->_sds);
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
        source.type = STL_TYPE__SDS;
        source.n_values = 10;
        source.values = zmalloc(sizeof(StlEntry *) * source.n_values);

        // ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
        for (size_t i = 0; i < 10; ++i) {
            sds raw_entry = sdscatfmt(sdsempty(), "%U", i);
            source.values[i] = (StlEntry *) zmalloc(sizeof(StlEntry));
            stl_entry__init(source.values[i]);
            source.values[i]->value_case = STL_ENTRY__VALUE__SDS;
            source.values[i]->_sds = sds2proto(raw_entry);
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
            sds source_sds = proto2sds(source.values[i]->_sds);
            sds target_sds = proto2sds(target->values[i]->_sds);
            assert(
                sdscmp(source_sds, target_sds) == 0);
        }

        // Free protobuf-related data...
        for (size_t i = 0; i < source.n_values; ++i) {
            sds source_entry_sds = proto2sds(source.values[i]->_sds);
            sdsfree(source_entry_sds);
            zfree(source.values[i]->_sds);
            zfree(source.values[i]);
        }
        zfree(source.values);
        zfree(serialized);
        proto_vector__free_unpacked(target, NULL);
        serverLog(LL_DEBUG, "[ADDB_TEST][ProtoVector] SUCCESS");
    }

    addReply(c, shared.ok);
}

/*
 * testProtoVectorInterfaceCommand
 * Tests Protobuf-Vector user-friendly interface.
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTPROTOVECTORINTERFACE
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testProtoVectorInterfaceCommand(client *c) {
    {
        // ProtoVector Type [LONG]
        ProtoVector v;
        protoVectorTypeInit(&v, STL_TYPE_LONG);
        serverLog(LL_DEBUG, "[ADDB_TEST][PROTO_VECTOR][LONG] Test LONG type ProtoVector");
        assert(v.n_values == 0);
        assert(v.count == 0);
        const long values[] = { 1, 2, 3 };
        protoVectorAdd(&v, (void *) values[0]);
        protoVectorAdd(&v, (void *) values[1]);
        protoVectorAdd(&v, (void *) values[2]);
        assert(v.count == 3);
        for (size_t i = 0; i < v.count; ++i) {
            assert(values[i] == (long) protoVectorGet(&v, i));
        }
        // Pop test
        long popedDatum;
        popedDatum = (long) protoVectorPop(&v);
        assert(v.count == 2 && popedDatum == values[2]);
        protoVectorDelete(&v, 0);
        assert(v.count == 1);
        protoVectorDelete(&v, 0);
        assert(v.count == 0);
        protoVectorFreeDeep(&v);
    }
    {
        // ProtoVector Type [SDS]
        ProtoVector v;
        protoVectorTypeInit(&v, STL_TYPE_SDS);
        serverLog(LL_DEBUG, "[ADDB_TEST][PROTO_VECTOR][SDS] Test SDS type ProtoVector");
        assert(v.n_values == 0);
        assert(v.count == 0);
        const sds values[] = {
            sdsnew("TEST_VECTOR_SDS_VALUE_1"),
            sdsnew("TEST_VECTOR_SDS_VALUE_2"),
            sdsnew("TEST_VECTOR_SDS_VALUE_3"),
        };
        protoVectorAdd(&v, (void *) values[0]);
        protoVectorAdd(&v, (void *) values[1]);
        protoVectorAdd(&v, (void *) values[2]);
        assert(v.count == 3);
        for (size_t i = 0; i < v.count; ++i) {
            assert(sdscmp(values[i], (sds) protoVectorGet(&v, i)) == 0);
        }
        // Pop test
        sds popedDatum;
        popedDatum = (sds) protoVectorPop(&v);
        assert(v.count == 2 && sdscmp(popedDatum, values[2]) == 0);
        protoVectorDelete(&v, 0);
        assert(v.count == 1);
        protoVectorDelete(&v, 0);
        assert(v.count == 0);
        protoVectorFreeDeep(&v);
        sdsfree(popedDatum);
    }
    addReply(c, shared.ok);
}

/*
 * testProtoVectorSerializationCommand
 * Tests Protobuf-Vector user-friendly Serialization-Deserialization.
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTPROTOVECTORSERIALIZATION
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testProtoVectorSerializationCommand(client *c) {
    {
        // ProtoVector Type [LONG]
        ProtoVector v;
        protoVectorTypeInit(&v, STL_TYPE_LONG);
        serverLog(
            LL_DEBUG,
            "[ADDB_TEST][PROTO_VECTOR][LONG] Test LONG type ProtoVector Serialization"
        );
        const long values[] = { 1, 2, 3 };
        protoVectorAdd(&v, (void *) values[0]);
        protoVectorAdd(&v, (void *) values[1]);
        protoVectorAdd(&v, (void *) values[2]);
        // Serialization
        char *serialized = protoVectorSerialize(&v);
        // Deserialization
        ProtoVector *target;
        protoVectorDeserialize(serialized, &target);
        assert(v.type == target->type);
        assert(v.n_values == target->n_values);
        assert(v.count == target->count);
        for (size_t i = 0; i < v.count; ++i) {
            long source_entry = (long) protoVectorGet(&v, i);
            long target_entry = (long) protoVectorGet(target, i);
            assert(source_entry == target_entry);
        }
        protoVectorFreeDeep(&v);
        protoVectorFreeDeserialized(target);
        zfree(serialized);
    }
    {
        // ProtoVector Type [SDS]
        ProtoVector v;
        protoVectorTypeInit(&v, STL_TYPE_SDS);
        serverLog(
            LL_DEBUG,
            "[ADDB_TEST][PROTO_VECTOR][SDS] Test SDS type ProtoVector Serialzation"
        );
        assert(v.n_values == 0);
        assert(v.count == 0);
        const sds values[] = {
            sdsnew("TEST_VECTOR_SDS_VALUE_1"),
            sdsnew("TEST_VECTOR_SDS_VALUE_2"),
            sdsnew("TEST_VECTOR_SDS_VALUE_3"),
        };
        protoVectorAdd(&v, (void *) values[0]);
        protoVectorAdd(&v, (void *) values[1]);
        protoVectorAdd(&v, (void *) values[2]);
        // Serialization
        char *serialized = protoVectorSerialize(&v);
        // Deserialization
        ProtoVector *target;
        protoVectorDeserialize(serialized, &target);
        assert(v.type == target->type);
        assert(v.n_values == target->n_values);
        assert(v.count == target->count);
        for (size_t i = 0; i < v.count; ++i) {
            sds source_entry = (sds) protoVectorGet(&v, i);
            sds target_entry = (sds) protoVectorGet(target, i);
            assert(sdscmp(source_entry, target_entry) == 0);
        }
        protoVectorFreeDeep(&v);
        protoVectorFreeDeserialized(target);
        zfree(serialized);
    }
    addReply(c, shared.ok);
}

/*
 * testCmpSerializationTimeCommand
 * Tests compare serialization time Naive with Protobuf.
 * --- Parameters ---
 *  None
 *
 * --- Usage Examples ---
 *  Parameters:
 *      None
 *  Command:
 *      redis-cli> TESTCMPSERIALIZATIONTIME
 *  Results:
 *      redis-cli> OK (prints results to server logs)
 */
void testCmpSerializationTimeCommand(client *c) {
    {
        // Type [SDS]
        const size_t n = 300;
        sds *values = (sds *) zmalloc(sizeof(sds) * n);
        for (size_t i = 0; i < n; ++i) {
            values[i] = sdscatfmt(sdsempty(), "TEST_VECTOR_SDS_VALUE_%u", i);
        }

        typedef struct CmpSerializationTimeStat {
            long long naive_serialize_us;
            long long naive_deserialize_us;
            long long protobuf_serialize_us;
            long long protobuf_deserialize_us;
        } __stat;
        __stat time_statistics;

        long long start_us, end_us;

        // Naive - Serialization, Deserialization
        {
            Vector *v = (Vector *) zmalloc(sizeof(Vector));
            vectorTypeInit(v, STL_TYPE_SDS);
            for (size_t i = 0; i < n; ++i) {
                vectorAdd(v, (void *) sdsdup(values[i]));
            }

            // Serialization
            robj *v_obj = createObject(OBJ_VECTOR, v);

            ////// Time check //////
            start_us = ustime();
            char *serialized = VectorSerialize((void *) v_obj);
            end_us = ustime();
            time_statistics.naive_serialize_us = end_us - start_us;
            ////// Time check //////

            // Deserialization
            sds serialized_sds = sdsnew(serialized);
            zfree(serialized);
            Vector *deserialized;
            ////// Time check //////
            start_us = ustime();
            vectorDeserialize(serialized_sds, &deserialized);
            end_us = ustime();
            time_statistics.naive_deserialize_us = end_us - start_us;
            ////// Time check //////

            sdsfree(serialized_sds);
            vectorFreeDeep(deserialized);
            zfree(deserialized);
            decrRefCount(v_obj);
        }

        // ProtoVector - Serialization, Deserialization
        {
            ProtoVector v;
            protoVectorTypeInit(&v, STL_TYPE_SDS);
            for (size_t i = 0; i < n; ++i) {
                protoVectorAdd(&v, (void *) sdsdup(values[i]));
            }

            // Serialization
            ////// Time check //////
            start_us = ustime();
            char *serialized = protoVectorSerialize(&v);
            end_us = ustime();
            time_statistics.protobuf_serialize_us = end_us - start_us;
            ////// Time check //////

            // Deserialization
            ProtoVector *target;
            ////// Time check //////
            start_us = ustime();
            protoVectorDeserialize(serialized, &target);
            end_us = ustime();
            time_statistics.protobuf_deserialize_us = end_us - start_us;
            ////// Time check //////

            protoVectorFreeDeep(&v);
            protoVectorFreeDeserialized(target);
            zfree(serialized);
        }

        for (size_t i = 0; i < n; ++i) {
            sdsfree(values[i]);
        }
        zfree(values);

        // Prints statistics
        serverLog(LL_DEBUG, "[testCmpSerializationTimeTest] Statistics N: %zu", n);
        serverLog(LL_DEBUG, "================================================");
        serverLog(
            LL_DEBUG,
            "[NAIVE][SERIALIZE] elapsed time: %lld us",
            time_statistics.naive_serialize_us);
        serverLog(
            LL_DEBUG,
            "[NAIVE][DESERIALIZE] elapsed time: %lld us",
            time_statistics.naive_deserialize_us);
        serverLog(
            LL_DEBUG,
            "[PROTOBUF][SERIALIZE] elapsed time: %lld us",
            time_statistics.protobuf_serialize_us);
        serverLog(
            LL_DEBUG,
            "[PROTOBUF][DESERIALIZE] elapsed time: %lld us",
            time_statistics.protobuf_deserialize_us);
        serverLog(LL_DEBUG, "================================================");
    }
    addReply(c, shared.ok);
}