#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "entity.h"
#include "platform.h"
#include "vfs.h"

static void test_entity_manager(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    // Test allocation
    entity_manager_t* manager = entity_manager_new();
    assert_non_null(manager);

    // Test creation
    entity_t* first_entity = entity_manager_create(manager);
    assert_non_null(first_entity);
    assert_true(first_entity->id == 1);

    // Test id increment
    entity_t* second_entity = entity_manager_create(manager);
    assert_non_null(second_entity);
    assert_true(second_entity->id == 2);

    // Test getting a known-existant entity.
    entity_t* other = entity_manager_get(manager, 1);
    assert_non_null(other);
    assert_true(first_entity == other);

    // Test getting a known-nonexistant entity
    entity_t* noexist = entity_manager_get(manager, 3);
    assert_null(noexist);

    // Test entity deletion
    entity_manager_destroy(manager, 1);

    // Test getting a known-deleted entity
    entity_t* deleted = entity_manager_get(manager, 1);
    assert_null(deleted);

    // Test that we still have our second entity
    other = entity_manager_get(manager, 2);
    assert_non_null(other);
    assert_true(second_entity == other);

    // Test that double-deletion doesn't break anything - should trigger ASan
    entity_manager_destroy(manager, 1);

    entity_manager_delete(manager);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();

    assert_true(error_count() == 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_entity_manager),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
