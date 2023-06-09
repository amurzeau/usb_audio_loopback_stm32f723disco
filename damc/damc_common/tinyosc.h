/**
 * Copyright (c) 2015-2018, Martin Roth (mhroth@gmail.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define TINYOSC_TIMETAG_IMMEDIATELY 1L

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tosc_message {
	char* marker;      // the current write head
	char* buffer;      // the original message data (also points to the address)
	char* buffer_end;  // length of the buffer data
} tosc_message;

typedef struct tosc_message_const {
	const char* format;  // a pointer to the format field
	const char* marker;  // the current read head
	const char* buffer;  // the original message data (also points to the address)
	uint32_t len;        // length of the buffer data
} tosc_message_const;

typedef struct tosc_bundle {
	char* marker;        // the current write head (where the next message will be written)
	char* buffer;        // the original buffer
	uint32_t bufLen;     // the byte length of the original buffer
	uint32_t bundleLen;  // the byte length of the total bundle
} tosc_bundle;

typedef struct tosc_bundle_const {
	const char* marker;  // the current write head (where the next message will be written)
	const char* buffer;  // the original buffer
	uint32_t bufLen;     // the byte length of the original buffer
	uint32_t bundleLen;  // the byte length of the total bundle
} tosc_bundle_const;

/**
 * Returns true if the buffer refers to a bundle of OSC messages. False otherwise.
 */
bool tosc_isBundle(const char* buffer);

/**
 * Reads a buffer containing a bundle of OSC messages.
 */
void tosc_parseBundle(tosc_bundle_const* b, const char* buffer, const int len);

/**
 * Returns the timetag of an OSC bundle.
 */
uint64_t tosc_getTimetag(tosc_bundle_const* b);

/**
 * Parses the next message in a bundle. Returns true if successful.
 * False otherwise.
 */
bool tosc_getNextMessage(tosc_bundle_const* b, tosc_message_const* o);

/**
 * Returns a point to the address block of the OSC buffer.
 * This is also the start of the buffer.
 */
const char* tosc_getAddress(tosc_message_const* o);

/**
 * Returns a pointer to the format block of the OSC buffer.
 */
const char* tosc_getFormat(tosc_message_const* o);

/**
 * Returns the length in bytes of this message.
 */
uint32_t tosc_getLength(tosc_message_const* o);

/**
 * Returns the next 32-bit int. Does not check buffer bounds.
 */
int32_t tosc_getNextInt32(tosc_message_const* o);

/**
 * Returns the next 64-bit int. Does not check buffer bounds.
 */
int64_t tosc_getNextInt64(tosc_message_const* o);

/**
 * Returns the next 64-bit timetag. Does not check buffer bounds.
 */
uint64_t tosc_getNextTimetag(tosc_message_const* o);

/**
 * Returns the next 32-bit float. Does not check buffer bounds.
 */
float tosc_getNextFloat(tosc_message_const* o);

/**
 * Returns the next 64-bit float. Does not check buffer bounds.
 */
double tosc_getNextDouble(tosc_message_const* o);

/**
 * Returns the next string, or NULL if the buffer length is exceeded.
 */
const char* tosc_getNextString(tosc_message_const* o);

/**
 * Points the given buffer pointer to the next blob.
 * The len pointer is set to the length of the blob.
 * Returns NULL and 0 if the OSC buffer bounds are exceeded.
 */
void tosc_getNextBlob(tosc_message_const* o, const char** buffer, int* len);

/**
 * Returns the next set of midi bytes. Does not check bounds.
 * Bytes from MSB to LSB are: port id, status byte, data1, data2.
 */
const unsigned char* tosc_getNextMidi(tosc_message_const* o);

/**
 * Resets the read head to the first element.
 *
 * @return  The same tosc_message pointer.
 */
tosc_message_const* tosc_reset(tosc_message_const* o);

/**
 * Parse a buffer containing an OSC message.
 * The contents of the buffer are NOT copied.
 * The tosc_message struct only points at relevant parts of the original buffer.
 * Returns 0 if there is no error. An error code (a negative number) otherwise.
 */
int tosc_parseMessage(tosc_message_const* o, const char* buffer, const int len);

/**
 * Starts writing a bundle to the given buffer with length.
 */
void tosc_writeBundle(tosc_bundle* b, uint64_t timetag, char* buffer, const int len);

/**
 * Write a message to a bundle buffer. Returns the number of bytes written.
 */
uint32_t tosc_writeNextMessage(tosc_bundle* b, const char* address, const char* format, ...);

/**
 * Returns the length in bytes of the bundle.
 */
uint32_t tosc_getBundleLength(tosc_bundle* b);

uint32_t tosc_writeMessageHeader(
    tosc_message* osc, const char* address, const char* format, char* buffer, const int len);

uint32_t tosc_writeNextInt32(tosc_message* o, int32_t value);

uint32_t tosc_writeNextInt64(tosc_message* o, int64_t value);

uint32_t tosc_writeNextTimetag(tosc_message* o, uint64_t value);

uint32_t tosc_writeNextFloat(tosc_message* o, float value);

uint32_t tosc_writeNextDouble(tosc_message* o, double value);

uint32_t tosc_writeNextString(tosc_message* o, const char* value);

uint32_t tosc_writeNextStringView(tosc_message* o, const char* value, int size);

uint32_t tosc_writeNextBlob(tosc_message* o, const char* buffer, int len);

uint32_t tosc_writeNextMidi(tosc_message* o, const unsigned char value[4]);

/**
 * Returns the length in bytes of the message.
 */
uint32_t tosc_getMessageLength(tosc_message* o);

/**
 * Writes an OSC packet to a buffer. Returns the total number of bytes written.
 * The entire buffer is cleared before writing.
 */
uint32_t tosc_writeMessage(char* buffer, const int len, const char* address, const char* fmt, ...);

/**
 * A convenience function to (non-destructively) print a buffer containing
 * an OSC message to stdout.
 */
void tosc_printOscBuffer(const char* buffer, const int len);

/**
 * A convenience function to (non-destructively) print a pre-parsed OSC message
 * to stdout.
 */
void tosc_printMessage(tosc_message_const* o);

#ifdef __cplusplus
}
#endif
