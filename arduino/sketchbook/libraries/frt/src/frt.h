/*
 * Copyright (c) 2018 Flössie <floessie.mail@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
//#include <STM32FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

namespace frt
{

	namespace detail {

		inline void yieldFromIsr() __attribute__((always_inline));

		void yieldFromIsr()
		{
			#ifdef portYIELD_FROM_ISR
			portYIELD_FROM_ISR();
			#else
				#ifdef portEND_SWITCHING_ISR
					portEND_SWITCHING_ISR();
				#else
					taskYIELD();
				#endif
			#endif
		}

	}

	template<typename T, unsigned int STACK_SIZE = configMINIMAL_STACK_SIZE * sizeof(StackType_t)>
	class Task
	{
	public:
		Task() :
			running(false),
			do_stop(false),
			handle(nullptr)
		{
		}

		~Task()
		{
			stop();
		}

		explicit Task(const Task& other) = delete;
		Task& operator =(const Task& other) = delete;

		bool start(unsigned char priority = 0, const char* name = "")
		{
			if (priority >= configMAX_PRIORITIES) {
				priority = configMAX_PRIORITIES - 1;
			}

#if configSUPPORT_STATIC_ALLOCATION > 0
			handle = xTaskCreateStatic(
				entryPoint,
				name,
				STACK_SIZE / sizeof(StackType_t),
				this,
				priority,
				stack,
				&state
			);
			return handle;
#else
			return
				xTaskCreate(
					entryPoint,
					name,
					STACK_SIZE / sizeof(StackType_t),
					this,
					priority,
					&handle
				) == pdPASS;
#endif
		}

		bool stop()
		{
			return stop(false);
		}

		bool stopFromIdleTask()
		{
			return stop(true);
		}

		bool isRunning() const
		{
			taskENTER_CRITICAL();
			const bool res = running;
			taskEXIT_CRITICAL();

			return res;
		}

		unsigned int getUsedStackSize() const
		{
			return STACK_SIZE - uxTaskGetStackHighWaterMark(handle) * sizeof(StackType_t);
		}

		void post()
		{
			xTaskNotifyGive(handle);
		}

		void preparePostFromInterrupt()
		{
			higher_priority_task_woken = 0;
		}

		void postFromInterrupt()
		{
			vTaskNotifyGiveFromISR(handle, &higher_priority_task_woken);
		}

		void finalizePostFromInterrupt() __attribute__((always_inline))
		{
			if (higher_priority_task_woken) {
				detail::yieldFromIsr();
			}
		}

	protected:
		void yield()
		{
			taskYIELD();
		}

		void msleep(unsigned int msecs)
		{
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;

			vTaskDelay(max(1, ticks));
		}

		void msleep(unsigned int msecs, unsigned int& remainder)
		{
			msecs += remainder;
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;
			remainder = msecs % portTICK_PERIOD_MS * static_cast<bool>(ticks);

			vTaskDelay(max(1, ticks));
		}

		void wait()
		{
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}

		bool wait(unsigned int msecs)
		{
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;

			return ulTaskNotifyTake(pdTRUE, max(1, ticks));
		}

		bool wait(unsigned int msecs, unsigned int& remainder)
		{
			msecs += remainder;
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;
			remainder = msecs % portTICK_PERIOD_MS * static_cast<bool>(ticks);

			if (ulTaskNotifyTake(pdTRUE, max(1, ticks))) {
				remainder = 0;
				return true;
			}

			return false;
		}

		void beginCriticalSection() __attribute__((always_inline))
		{
			taskENTER_CRITICAL();
		}

		void endCriticalSection() __attribute__((always_inline))
		{
			taskEXIT_CRITICAL();
		}

	private:
		bool stop(bool from_idle_task)
		{
			if (!handle) {
				return false;
			}

			taskENTER_CRITICAL();
			do_stop = true;
			while (running) {
				taskEXIT_CRITICAL();
				if (!from_idle_task) {
					vTaskDelay(1);
				} else {
					taskYIELD();
				}
				taskENTER_CRITICAL();
			}
			taskEXIT_CRITICAL();

			return true;
		}

		static void entryPoint(void* data)
		{
			Task* const self = static_cast<Task*>(data);

			bool do_stop;

			taskENTER_CRITICAL();
			self->running = true;
			do_stop = self->do_stop;
			taskEXIT_CRITICAL();

			while (!do_stop && static_cast<T*>(self)->run()) {
				taskENTER_CRITICAL();
				do_stop = self->do_stop;
				taskEXIT_CRITICAL();
			}

			taskENTER_CRITICAL();
			self->do_stop = false;
			self->running = false;
			taskEXIT_CRITICAL();

			const TaskHandle_t handle_copy = self->handle;
			self->handle = nullptr;

			vTaskDelete(handle_copy);
		}

		volatile bool running;
		volatile bool do_stop;
		TaskHandle_t handle;
		BaseType_t higher_priority_task_woken;
#if configSUPPORT_STATIC_ALLOCATION > 0
		StackType_t stack[STACK_SIZE / sizeof(StackType_t)];
		StaticTask_t state;
#endif
	};

	class Mutex final
	{
	public:
		Mutex() :
			handle(
#if configSUPPORT_STATIC_ALLOCATION > 0
				xSemaphoreCreateMutexStatic(&buffer)
#else
				xSemaphoreCreateMutex()
#endif
			)
		{
		}

		~Mutex()
		{
			vSemaphoreDelete(handle);
		}

		explicit Mutex(const Mutex& other) = delete;
		Mutex& operator =(const Mutex& other) = delete;

		void lock()
		{
			xSemaphoreTake(handle, portMAX_DELAY);
		}

		void unlock()
		{
			xSemaphoreGive(handle);
		}

	private:
		SemaphoreHandle_t handle;
#if configSUPPORT_STATIC_ALLOCATION > 0
		StaticSemaphore_t buffer;
#endif
	};

	class Semaphore final
	{
	public:
		Semaphore(bool counting = false) :
			handle(
#if configSUPPORT_STATIC_ALLOCATION > 0
				counting
					? xSemaphoreCreateCountingStatic(static_cast<UBaseType_t>(-1), 0, &buffer)
					: xSemaphoreCreateBinaryStatic(&buffer)
#else
				counting
					? xSemaphoreCreateCounting(static_cast<UBaseType_t>(-1), 0)
					: xSemaphoreCreateBinary()
#endif
			)
		{
		}

		~Semaphore()
		{
			vSemaphoreDelete(handle);
		}

		explicit Semaphore(const Semaphore& other) = delete;
		Semaphore& operator =(const Semaphore& other) = delete;

		void wait()
		{
			xSemaphoreTake(handle, portMAX_DELAY);
		}

		bool wait(unsigned int msecs)
		{
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;

			return xSemaphoreTake(handle, max(1, ticks)) == pdTRUE;
		}

		bool wait(unsigned int msecs, unsigned int& remainder)
		{
			msecs += remainder;
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;
			remainder = msecs % portTICK_PERIOD_MS * static_cast<bool>(ticks);

			if (xSemaphoreTake(handle, max(1, ticks)) == pdTRUE) {
				remainder = 0;
				return true;
			}

			return false;
		}

		void post()
		{
			xSemaphoreGive(handle);
		}

		void preparePostFromInterrupt()
		{
			higher_priority_task_woken = 0;
		}

		void postFromInterrupt()
		{
			xSemaphoreGiveFromISR(handle, &higher_priority_task_woken);
		}

		void finalizePostFromInterrupt() __attribute__((always_inline))
		{
			if (higher_priority_task_woken) {
				detail::yieldFromIsr();
			}
		}

	private:
		SemaphoreHandle_t handle;
		BaseType_t higher_priority_task_woken;
#if configSUPPORT_STATIC_ALLOCATION > 0
		StaticSemaphore_t buffer;
#endif
	};

	template<typename T, unsigned int ITEMS>
	class Queue final
	{
	public:
		Queue() :
			handle(
#if configSUPPORT_STATIC_ALLOCATION > 0
				xQueueCreateStatic(ITEMS, sizeof(T), buffer, &state)
#else
				xQueueCreate(ITEMS, sizeof(T))
#endif
			)
		{
		}

		~Queue()
		{
			vQueueDelete(handle);
		}

		explicit Queue(const Queue& other) = delete;
		Queue& operator =(const Queue& other) = delete;

		unsigned int getFillLevel() const
		{
			return ITEMS - uxQueueSpacesAvailable(handle);
		}

		void push(const T& item)
		{
			xQueueSend(handle, &item, portMAX_DELAY);
		}

		bool push(const T& item, unsigned int msecs)
		{
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;

			return xQueueSend(handle, &item, max(1, ticks)) == pdTRUE;
		}

		bool push(const T& item, unsigned int msecs, unsigned int& remainder)
		{
			msecs += remainder;
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;
			remainder = msecs % portTICK_PERIOD_MS * static_cast<bool>(ticks);

			if (xQueueSend(handle, &item, max(1, ticks)) == pdTRUE) {
				remainder = 0;
				return true;
			}

			return false;
		}

		void preparePushFromInterrupt()
		{
			higher_priority_task_woken_from_push = 0;
		}

		bool pushFromInterrupt(const T& item)
		{
			return xQueueSendFromISR(handle, &item, &higher_priority_task_woken_from_push) == pdTRUE;
		}

		void finalizePushFromInterrupt() __attribute__((always_inline))
		{
			if (higher_priority_task_woken_from_push) {
				detail::yieldFromIsr();
			}
		}

		void pop(T& item)
		{
			xQueueReceive(handle, &item, portMAX_DELAY);
		}

		bool pop(T& item, unsigned int msecs)
		{
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;

			return xQueueReceive(handle, &item, max(1, ticks)) == pdTRUE;
		}

		bool pop(T& item, unsigned int msecs, unsigned int& remainder)
		{
			msecs += remainder;
			const TickType_t ticks = msecs / portTICK_PERIOD_MS;
			remainder = msecs % portTICK_PERIOD_MS * static_cast<bool>(ticks);

			if (xQueueReceive(handle, &item, max(1, ticks)) == pdTRUE) {
				remainder = 0;
				return true;
			}

			return false;
		}

		void preparePopFromInterrupt()
		{
			higher_priority_task_woken_from_pop = 0;
		}

		bool popFromInterrupt(const T& item)
		{
			return xQueueReceiveFromISR(handle, &item, &higher_priority_task_woken_from_pop);
		}

		void finalizePopFromInterrupt() __attribute__((always_inline))
		{
			if (higher_priority_task_woken_from_pop) {
				detail::yieldFromIsr();
			}
		}

	private:
		QueueHandle_t handle;
		BaseType_t higher_priority_task_woken_from_push;
		BaseType_t higher_priority_task_woken_from_pop;
#if configSUPPORT_STATIC_ALLOCATION > 0
		uint8_t buffer[ITEMS * sizeof(T)];
		StaticQueue_t state;
#endif
	};

}
