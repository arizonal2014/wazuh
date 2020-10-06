/*
 * Copyright (C) 2015-2020, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "shared.h"

static void (*callback_ptr)(void) = NULL;
static w_linked_queue_t *queue_ptr = NULL; // Local ptr to queue
/****************SETUP/TEARDOWN******************/
void callback_queue_push_ex() {
    int *ptr = malloc(sizeof(int));
    *ptr = 0;
    linked_queue_push_ex(queue_ptr, ptr);   
}

int setup_queue(void **state) {
    w_linked_queue_t *queue = linked_queue_init();
    *state = queue;
    queue_ptr = queue;
    return 0;
}

int teardown_queue(void **state) {
    w_linked_queue_t *queue = *state;
    int *data = linked_queue_pop(queue);
    while(data) {
        os_free(data);
        data = linked_queue_pop(queue);
    }
    linked_queue_free(queue);
    queue_ptr = NULL;
    return 0;
}

int setup_queue_with_values(void **state) {
    w_linked_queue_t *queue = linked_queue_init();
    *state = queue;
    int *ptr = malloc(sizeof(int));
    *ptr = 3;
    linked_queue_push(queue, ptr);
    int *ptr2 = malloc(sizeof(int));
    *ptr2 = 5;
    linked_queue_push(queue, ptr2);
    queue_ptr = queue;
    return 0;
}


/*****************WRAPS********************/
int __wrap_pthread_mutex_lock(pthread_mutex_t *mutex) {
    check_expected_ptr(mutex);
    return 0;
}

int __wrap_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    check_expected_ptr(mutex);
    return 0;
}

int __wrap_pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex) {
    check_expected_ptr(cond);
    check_expected_ptr(mutex);
    // callback function to avoid infinite loops when testing 
    if (callback_ptr)
        callback_ptr();
    return 0;
}

int __wrap_pthread_cond_signal(pthread_cond_t *cond) {
    check_expected_ptr(cond);
    return 0;
}

/****************TESTS***************************/
void test_linked_queue_push(void **state) {
    w_linked_queue_t *queue = *state;
    int *ptr = malloc(sizeof(int));
    *ptr = 2;
    linked_queue_push(queue, ptr);
    assert_ptr_equal(queue->first->data, ptr);
    assert_ptr_equal(queue->last->data, ptr);
    int *ptr2 = malloc(sizeof(int));
    *ptr2 = 5;
    linked_queue_push(queue, ptr2);
    assert_ptr_equal(queue->first->data, ptr);
    assert_ptr_equal(queue->last->data, ptr2);
    assert_int_equal(queue->elements, 2);
}

void test_linked_queue_push_ex(void **state) {
    w_linked_queue_t *queue = *state;
    int *ptr = malloc(sizeof(int));
    *ptr = 2;
    expect_value_count(__wrap_pthread_mutex_lock, mutex, &queue->mutex, 2);
    expect_value_count(__wrap_pthread_cond_signal, cond, &queue->available, 2);
    expect_value_count(__wrap_pthread_mutex_unlock, mutex, &queue->mutex, 2);
    linked_queue_push_ex(queue, ptr);
    assert_ptr_equal(queue->first->data, ptr);
    assert_ptr_equal(queue->last->data, ptr);
    int *ptr2 = malloc(sizeof(int));
    *ptr2 = 5;
    linked_queue_push_ex(queue, ptr2);
    assert_ptr_equal(queue->first->data, ptr);
    assert_ptr_equal(queue->last->data, ptr2);
    assert_int_equal(queue->elements, 2);
}


void test_linked_pop_empty(void **state) {
    w_linked_queue_t *queue = *state;
    void *data = linked_queue_pop(queue);
    assert_ptr_equal(data, NULL);
}

void test_linked_pop(void **state) {
    w_linked_queue_t *queue = *state;
    assert_int_equal(queue->elements, 2);
    int *data = linked_queue_pop(queue);
    assert_int_equal(queue->elements, 1);
    assert_ptr_not_equal(data, NULL);
    assert_int_equal(*data, 5);
    data = linked_queue_pop(queue);
    assert_int_equal(queue->elements, 0);
    assert_ptr_not_equal(data, NULL);
    assert_int_equal(*data, 3);
    // Check queue is now empty
    assert_ptr_equal(queue->first, NULL);
    assert_ptr_equal(queue->last, NULL);
}

void test_linked_pop_ex(void **state) {
    w_linked_queue_t *queue = *state;
    assert_int_equal(queue->elements, 2);
    expect_value_count(__wrap_pthread_mutex_lock, mutex, &queue->mutex, 4);
    expect_value_count(__wrap_pthread_mutex_unlock, mutex, &queue->mutex, 4);
    int *data = linked_queue_pop_ex(queue);
    assert_int_equal(queue->elements, 1);
    assert_ptr_not_equal(data, NULL);
    assert_int_equal(*data, 5);
    data = linked_queue_pop_ex(queue);
    assert_int_equal(queue->elements, 0);
    assert_ptr_not_equal(data, NULL);
    assert_int_equal(*data, 3);
    // Check queue is now empty
    assert_ptr_equal(queue->first, NULL);
    assert_ptr_equal(queue->last, NULL);
    expect_value(__wrap_pthread_cond_wait, cond, &queue->available);
    expect_value(__wrap_pthread_cond_wait, mutex, &queue->mutex);
    expect_value(__wrap_pthread_cond_signal, cond, &queue->available);
    callback_ptr = callback_queue_push_ex;
    data = linked_queue_pop_ex(queue);

    
}
/************************************************/
int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_linked_queue_push, setup_queue, teardown_queue),
        cmocka_unit_test_setup_teardown(test_linked_queue_push_ex, setup_queue, teardown_queue),
        cmocka_unit_test_setup_teardown(test_linked_pop_empty, setup_queue, teardown_queue),
        cmocka_unit_test_setup_teardown(test_linked_pop, setup_queue_with_values, teardown_queue),
        cmocka_unit_test_setup_teardown(test_linked_pop_ex, setup_queue_with_values, teardown_queue),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
