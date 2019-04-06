#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "test.h"

#include "error.h"
#include "proto.h"

static void test_proto(void** state) {
    proto_container_t* protos = proto_container_new();
    assert_true(protos != NULL);

    proto_t* proto = calloc(1, sizeof(proto_t));
    proto->type = PROTO_PIECE;

    proto_container_push(protos, proto);

    proto_container_delete(protos);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_proto),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
