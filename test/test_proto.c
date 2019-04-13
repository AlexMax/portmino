#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "test.h"

#include "proto.h"

static void test_proto(void** state) {
    proto_container_t* protos = proto_container_new();
    assert_true(protos != NULL);

    for (size_t i = 0;i < 40;i++) {
        piece_config_t* piece = calloc(1, sizeof(piece_config_t));
        assert_true(piece != NULL);
        proto_t* proto = proto_new(MINO_PROTO_PIECE, piece, free);
        assert_true(proto != NULL);
        bool ok = proto_container_push(protos, proto);
        assert_true(ok);
    }

    proto_container_delete(protos);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_proto),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
