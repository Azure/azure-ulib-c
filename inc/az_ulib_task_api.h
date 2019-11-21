// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_TASK_API_H
#define AZ_ULIB_TASK_API_H

#include "az_ulib_base.h"
#include "az_ulib_config.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_pal_os.h"
#include "az_ulib_result.h"
#include "internal/az_ulib_task.h"

#ifdef __cplusplus
#include <stdint.h>
extern "C" {
#include <cstdint>
#endif /* __cplusplus */

/**
 * @file    az_ulib_task_api.h
 *
 * @brief   This is the component that handle all tasks in the system.
 *
 * Tasks are calls that will be executed in background and will return its result by a callback in
 * some point in the future.
 *
 * A Task may be created to call both synchronous and asynchronous methods published on the IPC, and
 * it may or may not use an OS thread on this process. It is important to define the possible uses
 * of the task in a system. So, lets first classify a system using the following tree.
 *
 * @code
 *                                                System
 *                                                 /  \
 *                                 +--------------+    +---------------------------+
 *                                /                                                 \
 *                               OS                                                non-OS
 *                              /  \                                                /  \
 *              +--------------+    +----------------+                        +----+    +----+
 *             /                                      \                      /                \
 *           tick                                   non-tick                tick            non-tick
 *           /  \                                     /  \
 *     +----+    +----+                         +----+    +----+
 *    /                \                       /                \
 * thread          non-thread               thread          non-thread
 * @endcode
 *
 * Where:
 *    - **OS**:     Means that the system contains an OS support with at least a scheduler that can
 *                  handle tasks, and tools like locks and thread safe queues.
 *    - **tick**:   Means that the system may have a infinite loop, or a timer interrupt that will
 *                  pooling the code, no matter if there are anything to do or not.
 *    - **thread**: Means that the developer will use more than one thread in the system.
 *
 * This tree shows that the task may work on a variant types of systems, from the ones that contains
 * a full-blown OS, where all operations will run on threads and may use blocking calls, to the ones
 * that will run on battery, without OS or any kind of `tick`, where the system must sleep as much
 * as possible.
 *
 * @note  A system without OS still have the ability to run tasks in background (mainly using a
 *        secondary hardware), and still have concurrency problems, mainly because of the hardware
 *        interruptions.
 */

/**
 * @brief Task handle.
 */
typedef struct az_ulib_task_tag {
  _az_ulib_task az_private;
} az_ulib_task;

/**
 * @brief       Callback prototype for task result.
 */
typedef void (*az_ulib_task_result_callback)(az_ulib_task* task_handle);

/**
 * @brief       Initialize a new task.
 *
 * Task is the entity that will care a job that will be executed in the future, and will return its
 * result in some point it the future, by calling the provided callback.
 *
 * A task may handle an asynchronous call (the ones that relies on an hardware interruption to start
 * the end of call process), a synchronous call running in a separated thread, or a combination of
 * both, which is a asynchronous call started in a separated thread.
 *
 * All task calls will be done through an IPC mechanism, it will help the Task to:
 *  1) Have a standard way to create methods to run in a Task, for both sync and async ones.
 *  2) Protect the task call to avoid any call to a method that does not exist.
 *  3) Respect the correct elevation of the methods.
 *
 * @param[out]  tasl_handle       The pointer to #az_ulib_task that will care the task up to the
 *                                end of the execution of the job, when, in some point in the
 *                                future, the result_callback is called or the
 *                                az_ulib_task_get_result() returns some end result, the ones
 *                                different than #AZ_ULIB_WAITING_FOR_ACTIVATION, #AZ_ULIB_RUNNING,
 *                                and #AZ_ULIB_WAITING_FOR_COMPLETION. The caller shall allocated
 *                                this memory in a way that it will be valid during this time. It
 *                                cannot be `NULL`.
 * @param[in]   interface_handle  The #az_ulib_ipc_interface_handle with the interface handle. It
 *                                cannot be `NULL`. Call az_ulib_ipc_try_get_interface() to get
 *                                the interface handle. The Task will get its own instance of the
 *                                interface, so the caller may release it immediately after the
 *                                Task is initialized.
 * @param[in]   method_index      The #az_ulib_action_index with the method handle.
 * @param[in]   model_in          The `const void *const` that points to the memory with the
 *                                input model content. It is part of the contract defined by the
 *                                publisher of the interface, the Task will not validate it. The
 *                                content of the model in shall be valid during the execution of
 *                                the Task, and may be released in some point in the future, when
 *                                the result_callback is called or the az_ulib_task_get_result()
 *                                returns some end result, the ones different than
 *                                #AZ_ULIB_WAITING_FOR_ACTIVATION, #AZ_ULIB_RUNNING, and
 *                                #AZ_ULIB_WAITING_FOR_COMPLETION.
 * @param[out]  model_out         The `const void *` that points to the memory where the target
 *                                method shall store the result of the call. It is part of the
 *                                contract defined by the publisher of the interface, the Task
 *                                will not validate it. This memory shall be valid up to some
 *                                point in the future, when the result_callback is called or the
 *                                az_ulib_task_get_result() returns some end result, the ones
 *                                different than #AZ_ULIB_WAITING_FOR_ACTIVATION, #AZ_ULIB_RUNNING,
 *                                and #AZ_ULIB_WAITING_FOR_COMPLETION.
 * @param[in]   result_callback   The #az_ulib_action_result_callback that points to the method
 *                                that Task shall call when the asynchronous call ends its
 *                                execution. This callback is optional and may be `NULL`. If the
 *                                callback is `NULL`, the Task will make this call *fire and
 *                                forget*. If this callback is `NULL`, the caller may still
 *                                monitor the state of the Task by calling
 *                                az_ulib_task_get_result().
 * @param[in]   queue             The #az_ulib_pal_os_queue that identify the queue that the task
 *                                should use to call the callback. It can be `NULL`. If `NULL`,
 *                                the task will call the callback directly in the current call
 *                                stack.
 *
 * @return The #az_ulib_result with the result of the Task initialization.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task initialization got success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 */
static inline az_ulib_result az_ulib_task_init(
    az_ulib_task* task_handle,
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out,
    az_ulib_action_result_callback result_callback,
    az_ulib_pal_os_queue queue) {
#ifdef AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT
  return _az_ulib_task_init(
      task_handle,
      (_az_ulib_ipc_interface_handle)interface_handle,
      method_index,
      model_in,
      model_out,
      result_callback,
      queue);
#else
  return _az_ulib_task_init_no_contract(
      task_handle,
      (_az_ulib_ipc_interface_handle)interface_handle,
      method_index,
      model_in,
      model_in,
      result_callback,
      queue);
#endif /* AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT */
}

/**
 * @brief       Run a created task.
 *
 * This API will call the target method on the task, providing the `model_in` and the `model_out`
 * arguments plus the callback for the future result (if provided).
 *
 * The following diagram represents an asynchronous call in the time. The Publisher is the component
 * that published an interface and implements the methods on it, and the Consumer is the component
 * that will call the methods in the interface.
 *
 * @code
 * +---------------+         +---------------+         +---------------+         +---------------+
 * |    Consumer   |         |      Task     |         |      IPC      |         |   Publisher   |
 * +---------------+         +---------------+         +---------------+         +---------------+
 *         |                         |                         |                         |
 *         +-az_ulib_task_init(      |                         |                         |
 *         |   task,                 |                         |                         |
 *         |   my_interface,         |                         |                         |
 *         |   MY_METHOD_ASYNC_INDEX,|                         |                         |
 *         |   my_model_in,          |                         |                         |
 *         |   my_model_out,         |                         |                         |
 *         |   my_method_async_cb,   |                         |                         |
 *         |   NULL)---------------->|                         |                         |
 *         |                         |                         |                         |
 *         +----az_ulib_task_run(    |                         |                         |
 *         |      task)------------->|                         |                         |
 *         |                     +---+                         |                         |
 *         |                     | task->result <- AZ_ULIB_RUNNING                       |
 *         |                     +-->|                         |                         |
 *         |                         +-az_ulib_ipc_call_async( |                         |
 *         |                         |    task->interface_handle,                        |
 *         |                         |    task->method_index,  |                         |
 *         |                         |    task->model_in,      |                         |
 *         |                         |    task->model_out,     |                         |
 *         |                         |    complete_task_cb,    |                         |
 *         |                         |    (context)task)------>|                         |
 *         |                         |            +------------+                         |
 *         |                         |            |interface_handle->running_count++     |
 *         |                         |            |my_method_async <- interface_handle[method_index]
 *         |                         |            +----------->|                         |
 *         |                         |                         +-my_method_async(        |
 *         |                         |                         |   model_in, model_out,  |
 *         |                         |                         |   complete_task_cb,     |
 *         |                         |                         |   (context)task)------->|
 *         |                         |                         | +-----------------------+
 *         |                         |                         | |Execute the synchronous portion of
 *         |                         |                         | |the asynchronous method, starting
 *         |                         |                         | |the background process.
 *         |                         |                         | +---------------------->|
 *         |                         |                         |                         +---+
 *         |                         |                         |<---<AZ_ULIB_SUCCESS>----+   |b
 *         |                         |                  +------+                         |   |a
 *         |                         |                  | interface_handle->running_count--  |c
 *         |                         |                  +----->|                         |   |k
 *         |                         |<---<AZ_ULIB_SUCCESS>----+                         |   |g
 *         |                     +---+                         |                         |   |r
 *         |                     | task->result <- AZ_ULIB_WAITING_FOR_COMPLETION        |   |o
 *         |                     +-->|                         |                         |   |u
 *         |<---<AZ_ULIB_SUCCESS>----+                         |                         |   |n
 *         |                         |                         |                         |   |d
 *         |                                                                                 |
 *         |        consumer may do other stuffs while wait for the callback.                |
 *                                                                                           |
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                    Background ends an generates a hardware interruption (IRQ)             |
 *                                                                                       |<--+
 *         |                         |            complete_task_cb((context)task,  ------+
 *         |                         |<----------------- AZ_ULIB_SUCCESS, model_out)     |
 *         |                     +---+                         |                         |
 *         |                     | az_ulib_task_run(task)      |                         |
 *         |                     | task->result <- AZ_ULIB_SUCCESS                       |
 *         |                     +-->|                         |                         |
 *         |  my_method_async_cb( ---+                         |                         |
 *         |<-- task)                |                         |                         |
 *     +---+                         |                         |                         |
 *     | Use the information in the task.                      |                         |
 *     | release(task->model_in)     |                         |                         |
 *     | release(task->model_out)    |                         |                         |
 *     | release(task)               |                         |                         |
 *     +-->|                         |                         |                         |
 *         +------------------------>|                         |                         |
 *         |                         +----<AZ_ULIB_SUCCESS>--->|                         |
 *         |                         |                         +------------------------>|
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *         |                         |                         |                         |
 * @endcode
 *
 * @note  For a synchronous method, create a Task and run using this API is equivalent to directly
 *        call the method using az_ulib_ipc_call(). It is a blocking call and no callback will be
 *        called at the end of the execution.
 *
 * @param[out]  task_handle       The handle to #az_ulib_task with the task to be executed. It
 *                                cannot be `NULL`.
 *
 * @return The #az_ulib_result with the Task execution.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task executed with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method was unpublished. The
 *                                            `interface_handle` associated with this Task shall
 *                                            be released.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the target method cannot be executed at that
 *                                            moment.
 */
static inline az_ulib_result az_ulib_task_run(az_ulib_task* task_handle) {
#ifdef AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT
  return _az_ulib_task_run(task_handle);
#else
  return _az_ulib_task_run_no_contract(task_handle);
#endif /* AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT */
}

/**
 * @brief       Get the current state of the task.
 *
 * Returns the current state of the task execution. The task may be in an intermediate state or in a
 * final state. The intermediate state are the ones where the task and its artifacts shall not be
 * destroyed and the state itself may change in the next instruction. The final state are the
 * permanent ones, the task and its results may be used and destroyed. There are 2 intermediate
 * states, #AZ_ULIB_WAITING_FOR_ACTIVATION, #AZ_ULIB_RUNNING, and #AZ_ULIB_WAITING_FOR_COMPLETION,
 * all the other are final states.
 *
 * @param[out]  task_handle       The handle to #az_ulib_task with the task. It cannot be `NULL`.
 *
 * @return  The #az_ulib_result with the Task execution state. The result may be one of the ones
 *          described bellow plus the ones returned by the target method.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task ends its execution and the final result
 *                                            is in the `model_out`. The task and its argument are
 *                                            still necessary and shall be preserved.
 *  @retval #AZ_ULIB_WAITING_FOR_ACTIVATION   If the Task enqueued to be executed, but the
 *                                            execution didn't start yet. The task and its argument
 *                                            are still necessary and shall be preserved.
 *  @retval #AZ_ULIB_RUNNING                  If the Task is currently executing the target method.
 *                                            The task and its argument are still necessary and
 *                                            shall be preserved.
 *  @retval #AZ_ULIB_WAITING_FOR_COMPLETION   If the Task started the asynchronous method that
 *                                            started a background operation, and now, the task is
 *                                            waiting for the background to finish its execution.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments provided to the target
 *                                            method is invalid or if the provided task_handle is
 *                                            `NULL`. The task and its argument are not necessary
 *                                            anymore and may be free.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method was unpublished. The
 *                                            `interface_handle` associated with this Task shall
 *                                            be released. The task and its argument are not
 *                                            necessary anymore and may be free.
 *  @retval #AZ_ULIB_CANCELLED_ERROR          If the execution of the target method was successfully
 *                                            cancelled. The task and its argument are not
 *                                            necessary anymore and may be free.
 *  @retval #AZ_ULIB_TIMEOUT_ERROR            If the execution of the target method was successfully
 *                                            cancelled by timeout. The task and its argument are
 * not necessary anymore and may be free.
 */
static inline az_ulib_result az_ulib_task_get_result(az_ulib_task* task_handle) {
#ifdef AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT
  return _az_ulib_task_get_result(task_handle);
#else
  return _az_ulib_task_get_result_no_contract(task_handle);
#endif /* AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT */
}

/**
 * @brief       Try to cancel a task.
 *
 * Try to cancel a task. Depending of multiple factors, the task may or may not be cancelled. The
 * way that this API will try to cancel the task depend of the state of the task.
 *  - If the task is in a final state, it cannot be cancelled and this API will return
 *    #AZ_ULIB_PRECONDITION_ERROR.
 *  - If the state is #AZ_ULIB_WAITING_FOR_ACTIVATION, the task will cancel by mark it as a
 *    cancelled call, on this way, when the time comes to execute the target method, the task will
 *    just change its state to cancelled without execute the method.
 *  - If the state is #AZ_ULIB_RUNNING or #AZ_ULIB_WAITING_FOR_COMPLETION, the task will call the
 *    `cancel` method, if provided by the publisher, and return the cancel method result. If no
 *    `cancel` method was provided, this API will return #AZ_ULIB_NOT_SUPPORTED_ERROR.
 *
 * @note  A task cancelled with success will have a new state #AZ_ULIB_CANCELLED_ERROR.
 *
 * @param[out]  task_handle       The handle to #az_ulib_task to cancel. It cannot be `NULL`.
 *
 * @return  The #az_ulib_result with the Task execution. The result may be one of the ones
 *          described bellow plus the ones returned by the cancel method.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task cancelled with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the provided handle didn't correspond to a task.
 *  @retval #AZ_ULIB_PRECONDITION_ERROR       If the task is in a final state and cannot be
 *                                            cancelled anymore.
 *  @retval #AZ_ULIB_NOT_SUPPORTED_ERROR      If the published didn't provide a cancel method.
 */
static inline az_ulib_result az_ulib_task_cancel(az_ulib_task* task_handle) {
#ifdef AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT
  return _az_ulib_task_cancel(task_handle);
#else
  return _az_ulib_task_cancel_no_contract(task_handle);
#endif /* AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT */
}

/**
 * @brief       Run a created task and wait for its result.
 *
 * This API will call the target method on the task, providing the `model_in` and the `model_out`
 * arguments plus the callback for the future result (if provided), in the same way that
 * az_ulib_task_run(), but it will block the caller up to the point in time that the background
 * process ends. So, **this API converts an asynchronous call in a synchronous call**.
 *
 * The following diagram represents an asynchronous call in the time when executed by this API.
 * The Publisher is the component that published an interface and implements the methods on it,
 * and the Consumer is the component that will call the methods in the interface.
 *
 * @code
 * +---------------+         +---------------+         +---------------+         +---------------+
 * |    Consumer   |         |      Task     |         |      IPC      |         |   Publisher   |
 * +---------------+         +---------------+         +---------------+         +---------------+
 *         |                         |                         |                         |
 *         +-az_ulib_task_init(      |                         |                         |
 *         |   task,                 |                         |                         |
 *         |   my_interface,         |                         |                         |
 *         |   MY_METHOD_ASYNC_INDEX,|                         |                         |
 *         |   my_model_in,          |                         |                         |
 *         |   my_model_out,         |                         |                         |
 *         |   NULL,                 |                         |                         |
 *         |   NULL)---------------->|                         |                         |
 *         |                         |                         |                         |
 *         +-az_ulib_task_run_and_wait(                        |                         |
 *         |  task,                  |                         |                         |
 *         |  AZ_ULIB_WAIT_FOREVER)->|                         |                         |
 *         |                     +---+                         |                         |
 *         |                     | task->result <- AZ_ULIB_RUNNING                       |
 *         |                     +-->|                         |                         |
 *         |                         +-az_ulib_ipc_call_async( |                         |
 *         |                         |    task->interface_handle,                        |
 *         |                         |    task->method_index,  |                         |
 *         |                         |    task->model_in,      |                         |
 *         |                         |    task->model_out,     |                         |
 *         |                         |    complete_task_cb,    |                         |
 *         |                         |    (context)task)------>|                         |
 *         |                         |            +------------+                         |
 *         |                         |            |interface_handle->running_count++     |
 *         |                         |            |my_method_async <- interface_handle[method_index]
 *         |                         |            +----------->|                         |
 *         |                         |                         +-my_method_async(        |
 *         |                         |                         |   model_in, model_out,  |
 *         |                         |                         |   complete_task_cb,     |
 *         |                         |                         |   (context)task)------->|
 *         |                         |                         | +-----------------------+
 *         |                         |                         | |Execute the synchronous portion of
 *         |                         |                         | |the asynchronous method, starting
 *         |                         |                         | |the background process.
 *         |                         |                         | +---------------------->|
 *         |                         |                         |                         +---+
 *         |                         |                         |<---<AZ_ULIB_SUCCESS>----+   |b
 *         |                         |                  +------+                         |   |a
 *         |                         |                  | interface_handle->running_count--  |c
 *         |                         |                  +----->|                         |   |k
 *         |                         |<---<AZ_ULIB_SUCCESS>----+                         |   |g
 *         |                     +---+                         |                         |   |r
 *         |                     | task->result <- AZ_ULIB_WAITING_FOR_COMPLETION        |   |o
 *         |                     | Close semaphore.            |                         |   |u
 *         |                     | Wait for the semaphore.     |                         |   |n
 *         |                     |                             |                         |   |d
 *         |                     |                             |                         |   |
 *                                                                                           |
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *                    Background ends an generates a hardware interruption (IRQ)             |
 *                                                                                       |<--+
 *         |                     |                complete_task_cb((context)task,  ------+
 *         |                     |   |<----------------- AZ_ULIB_SUCCESS, model_out)     |
 *         |                     |   | az_ulib_task_run(task)                            |
 *         |                     |   | task->result <- AZ_ULIB_SUCCESS                   |
 *         |                     |   | release semaphore.                                |
 *         |                     |   +-------------------------------------------------->|
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *         |                     +-->|                         |                         |
 *         |<---<AZ_ULIB_SUCCESS>----+                         |                         |
 *     +---+                         |                         |                         |
 *     | Use the information in the task.                      |                         |
 *     | release(task->model_in)     |                         |                         |
 *     | release(task->model_out)    |                         |                         |
 *     | release(task)               |                         |                         |
 *     +-->|                         |                         |                         |
 *         |                         |                         |                         |
 * @endcode
 *
 * @note  For a synchronous method, create a Task and run using this API is equivalent to directly
 *        call the method using az_ulib_ipc_call().
 *
 *
 * @param[in]   task_handle       The handle to #az_ulib_task with the task to be executed. It
 *                                cannot be `NULL`.
 * @param[in]   wait_option_ms    The `uint32_t` with the maximum number of milliseconds the
 *                                function may wait to execute the target method:
 *                                        - #AZ_ULIB_NO_WAIT (0x00000000)
 *                                        - #AZ_ULIB_WAIT_FOREVER (0xFFFFFFFF)
 *                                        - timeout value (0x00000001 through 0xFFFFFFFE)
 *
 * @return The #az_ulib_result with the Task execution.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task executed with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method was unpublished. The
 *                                            `interface_handle` associated with this Task shall
 *                                            be released.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the target method cannot be executed at that
 *                                            moment.
 */
static inline az_ulib_result az_ulib_task_run_and_wait(
    az_ulib_task* task_handle,
    uint32_t wait_option_ms) {
#ifdef AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT
  return _az_ulib_task_run_and_wait(task_handle, wait_option_ms);
#else
  return _az_ulib_task_run_and_wait_no_contract(task_handle, wait_option_ms);
#endif /* AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT */
}

/**
 * @brief       Enqueue a created task to be executed in a thread.
 *
 * This API will enqueue the task in a dispatcher queue and return immediately. The tasks in a
 * dispatcher queue will be executed when the system calls the az_ulib_task_dispatcher(), in an
 * infinite loop or in one or multiple threads. After execute the target method handled by the
 * task, the az_ulib_task_dispatcher() will call the provided callback with the result. So,
 * **this API converts an synchronous call in a asynchronous call**.
 *
 * Before use this API, the system shall create an #az_ulib_pal_os_queue to be used as the
 * dispatcher queue, and call the az_ulib_task_dispatcher() in a infinite loop.
 *
 * @note  Please see the az_ulib_task_dispatcher() documentation to have more context about the
 *        block and non-block ways to dispatch the tasks.
 *
 * The following code is a sample how to call a synchronous method in a asynchronous way using a
 * task. To make the sample as simple as possible, it predicts success for all calls. On a real
 * implementation, all returns should be evaluated before proceed to the next one.
 *
 * @code
 *  static void result_callback(az_ulib_task* task_handle) {
 *
 *    //TODO: check the `task_handle->result` and use the returned information in the
 *    //      `task_handle->model_out`.
 *
 *    free(task_handle->model_in);
 *    free(task_handle->model_out);
 *    free(task_handle);
 *  }
 *
 *  az_ulib_ipc_interface_handle my_interface = NULL;
 *
 *  void my_sample() {
 *    az_ulib_task* task_handle = (az_ulib_task*)malloc(sizeof(az_ulib_task));
 *    foo_model_in* model_in = (foo_model_in*)malloc(sizeof(foo_model_in));
 *    foo_model_out* model_out = (foo_model_out*)malloc(sizeof(foo_model_out));
 *
 *    // Get the interface on the IPC.
 *    az_ulib_ipc_try_get_interface(FOO_NAME, FOO_VERSION, AZ_ULIB_VERSION_EQUALS_TO,
 *                                  &my_interface);
 *
 *    // Initialize the model_in.
 *    model_in->a = 10;
 *    model_in->b = 20;
 *
 *    // Initialize the new task that will handle the FOO call on my_interface.
 *    az_ulib_task_init(&task_handle, my_interface, FOO_INDEX, model_in, model_out, NULL, NULL);
 *
 *    // Enqueue the task, when it finishes, the dispatcher will call the result_callback.
 *    az_ulib_task_enqueue(my_queue, task_handle, result_callback);
 *
 *    // my_sample is free to keep doing other things.
 *  }
 * @endcode
 *
 * The following diagram represents the previews asynchronous call.
 * The Publisher is the component that published an interface and implements the methods on it,
 * and the Consumer is the component that will call the methods in the interface.
 *
 * On this context, execution in parallel means that the OS is doing the time sharing between the
 * thread where the consumer is running and the thread where task is running the enqueued task. The
 * context switch may happen between this 2 threads in any place of the code that is not protected.
 *
 * @code
 * +-----------+        +-----------+        +-----------+        +-----------+        +-----------+
 * |  Consumer |        |    Task   |        |   Thread  |        |    IPC    |        | Publisher |
 * +-----------+        +-----------+        +-----------+        +-----------+        +-----------+
 *       |                    |                    |                    |                    |
 *       +-az_ulib_task_init( |                    |                    |                    |
 *       |   task,            |                    |                    |                    |
 *       |   my_interface,    |                    |                    |                    |
 *       |   MY_METHOD_INDEX, |                    |                    |                    |
 *       |   my_model_in,     |                    |                    |                    |
 *       |   my_model_out,    |                    |                    |                    |
 *       |   NULL,            |                    |                    |                    |
 *       |   NULL)----------->|                    |                    |                    |
 *       |                    |                    |                    |                    |
 *       +-az_ulib_task_enqueue(                   |                    |                    |
 *       |  task, queue,      |                    |                    |                    |
 *       |  task_complete_cb)>|                    |                    |                    |
 *       |                +---+                    |                    |                    |
 *       |                | task->result <- AZ_ULIB_WAITING_FOR_ACTIVATION                   |
 *       |                | queue->add(task)       |                    |                    |
 *       |                +-->|                    |                    |                    |
 *       |<-<AZ_ULIB_SUCCESS>-+                    |                    |                    |
 *       |                                                                                   |
 *       |        consumer is free to do other stuffs in parallel with the task execution.   |
 *       |                                                                                   |
 *       |                    |                    +------+             |                    |
 *       |                    | az_ulib_task_dispatcher( -+             |                    |
 *       |                    |     queue,                |             |                    |
 *       |                    |<--- AZ_ULIB_WAIT_FOREVER) |             |                    |
 *       |                    |                    |<-----+             |                    |
 *       |                +---+                    |                    |                    |
 *       |                | new_task <- queue->dequeue(AZ_ULIB_WAIT_FOREVER)                 |
 *       |                | az_ulib_task_run(task) |                    |                    |
 *       |                | task->result <- AZ_ULIB_RUNNING             |                    |
 *       |                +-->|                    |                    |                    |
 *       |                    |                    |                    |                    |
 *       |                    +---------az_ulib_ipc_call(               |                    |
 *       |                    |           task->interface_hanle,        |                    |
 *       |                    |           task->method_index,           |                    |
 *       |                    |           task->model_in,               |                    |
 *       |                    |           task->model_out)------------->|                    |
 *       |                    |                    |    +---------------+                    |
 *       |                    |                    |    |interface_handle->running_count++   |
 *       |                    |                    |    |my_method <- interface_handle[method_index]
 *       |                    |                    |    +-------------->|                    |
 *       |                    |                    |                    +--my_method(        |
 *       |                    |                    |                    |   model_in,        |
 *       |                    |                    |                    |   model_out)------>|
 *       |                    |                    |          +------------------------------+
 *       |                    |                    |          |Execute the synchronous portion of
 *       |                    |                    |          |the my_method in a thread, in
 *       |                    |                    |          |parallel with the Consumer execution.
 *       |                    |                    |          +----------------------------->|
 *       |                    |                    |                    |<-<AZ_ULIB_SUCCESS>-+
 *       |                    |                    |    +---------------+                    |
 *       |                    |                    |    |interface_handle->running_count--   |
 *       |                    |                    |    +-------------->|                    |
 *       |                    |<-----------<AZ_ULIB_SUCCESS>------------+                    |
 *       |                +---+                    |                    |                    |
 *       |                | task->result <- AZ_ULIB_SUCCESS             |                    |
 *       |                +-->|                    |                    |                    |
 *       |                    +---+                |                    |                    |
 *       |   | task_complete_cb( -+                |                    |                    |
 *       |   |<- task)            |                |                    |                    |
 *       |   |Consumer handles callback in parallel with its own execution.                  |
 *       |   |release(task->model_in)              |                    |                    |
 *       |   |release(task->model_out)             |                    |                    |
 *       |   |release(task)       |                |                    |                    |
 *       |   +------------------->|                |                    |                    |
 *       |                    |<--+                |                    |                    |
 *       |                    +------------------->|                    |                    |
 *       |                    |                    |                    |                    |
 * @endcode
 *
 * A task that handles a asynchronous call may be enqueue as well, in this case, the synchronous
 * part of the call will be executed in a thread. The Consumer will receive 2 callbacks the first
 * one when the thread ends, and the second one when the background execution ends.
 *
 * The result callback (called at the end of the execution) may be called directly from the hardware
 * interruption or enqueued in the task to be called in a thread in the future. When the IPC calls
 * az_ulib_task_run() for a task that is in #AZ_ULIB_WAITING_FOR_COMPLETION, it will check the
 * value of the `queue` in the task, if it is `NULL`, this API will call the callback directly,
 * otherwise, it will change the task state to #AZ_ULIB_WAITING_FOR_COMPLETION_ACTIVATION and
 * enqueue the task in the provide queue.
 *
 * There is not relation between the 2 queues (call and callback), and one may be used without the
 * other, the consumer may call the method using a queue and request the callback directly or vice
 * versa. On the scenario where the task use queues on both directions, the queues may or may not
 * be the same. One possible scenario for different queues is when the Publisher is running in
 * kernel mode and the Consumer is in user mode. In this case, the first thread shall be consumed
 * by a thread in kernel mode and the other in the user mode.
 *
 * @param[out]  task_handle       The handle to #az_ulib_task with the task to be executed. It
 *                                cannot be `NULL`.
 * @param[in]   queue             The #az_ulib_pal_os_queue that identify the queue that the task
 *                                should use to call the target method. It cannot be `NULL`.
 * @param[in]   result_callback   The handle to #az_ulib_task_result_callback that points to the
 *                                function that should be called when the target method
 *                                finishes its synchronous execution. It can be `NULL`. If it is
 *                                `NULL`, no callback will be called.
 *
 * @return The #az_ulib_result with the Task execution.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task was enqueued with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target queue does not exist.
 */
static inline az_ulib_result az_ulib_task_enqueue(
    az_ulib_task* task_handle,
    az_ulib_pal_os_queue queue,
    az_ulib_task_result_callback result_callback) {
#ifdef AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT
  return _az_ulib_task_enqueue(task_handle, queue, result_callback);
#else
  return _az_ulib_task_enqueue_no_contract(task_handle, queue, result_callback);
#endif /* AZ_ULIB_CONFIG_TASK_VALIDATE_CONTRACT */
}

/**
 * @brief       Dispatch tasks in a queue.
 *
 * This API will dequeue tasks from the queue and run it using the az_ulib_task_run(). It was
 * created in a way to contemplate all the cases described by the tree in #az_ulib_task_api, where
 * the system may or may not have OS, tick, or thread.
 *
 * The dispatcher will bypass the wait option in the `dequeue` API of the `queue`. If properly
 * implemented, the queue will wait for new tasks using a semaphore, making this wait very efficient
 * and not over-stressing the CPU.
 *
 * @param[in]   queue             The #az_ulib_pal_os_queue that identify the queue that the task
 *                                should use to call the callback. It can be `NULL`. If `NULL`,
 *                                the task will call the callback directly in the current call
 *                                stack.
 * @param[in]   wait_option_ms    The `uint32_t` with the maximum number of milliseconds the
 *                                function may wait for a new task in the queue:
 *                                        - #AZ_ULIB_NO_WAIT (0x00000000)
 *                                        - #AZ_ULIB_WAIT_FOREVER (0xFFFFFFFF)
 *                                        - timeout value (0x00000001 through 0xFFFFFFFE)
 *
 * @return The #az_ulib_result with the result of the Task initialization.
 *  @retval #AZ_ULIB_SUCCESS                  If the Task initialization got success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_TIMEOUT_ERROR            If there was no new task in the queue thought the
 *                                            provided waiting time.
 */
static inline az_ulib_result az_ulib_task_dispatcher(
    az_ulib_pal_os_queue queue,
    uint32_t wait_option_ms) {
  return _az_ulib_task_dispatcher(queue, wait_option_ms);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_TASK_API_H */
