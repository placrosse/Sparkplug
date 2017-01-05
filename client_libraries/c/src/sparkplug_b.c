#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <sparkplug_b.h>
#include <sparkplug_b.pb.h>

// Global vars
uint64_t seq;

int grow_array(com_cirruslink_sparkplug_protobuf_Payload_Metric **metric_array, int current_size, int num_new_elems) {
        const int total_size = current_size + num_new_elems;
        com_cirruslink_sparkplug_protobuf_Payload_Metric *temp = (com_cirruslink_sparkplug_protobuf_Payload_Metric*)realloc(*metric_array,
                                                                 (total_size * sizeof(com_cirruslink_sparkplug_protobuf_Payload_Metric)));

        if (temp == NULL) {
                printf("Cannot allocate more memory.\n");
                return 0;
        } else {
                *metric_array = temp;
        }

        return total_size;
}

void add_metric(com_cirruslink_sparkplug_protobuf_Payload *payload,
			const char *name,
			bool has_alias,
			uint64_t alias,
			uint64_t datatype,
			bool is_historical,
			bool is_transient,
			bool is_null,
			const void *value,
			size_t size_of_value) {

	int size = payload->metrics_count;
	if (size == 0) {
		payload->metrics = (com_cirruslink_sparkplug_protobuf_Payload_Metric *) calloc(1, sizeof(com_cirruslink_sparkplug_protobuf_Payload_Metric));
		if(payload->metrics == NULL) {
			printf("Cannot allocate initial memory for data\n");
		} else {
			size = 1;
		}
	} else {
		size = grow_array(&payload->metrics, size, 1);
	}

	payload->metrics[size-1].name = (char *)malloc((strlen(name)+1)*sizeof(char));
	strcpy(payload->metrics[size-1].name, name);
	payload->metrics[size-1].has_alias = has_alias;
	if (has_alias) {
		payload->metrics[size-1].alias = alias;
	}
	payload->metrics[size-1].has_timestamp = true;
	payload->metrics[size-1].timestamp = get_current_timestamp();
	payload->metrics[size-1].has_datatype = true;
	payload->metrics[size-1].datatype = datatype;
	payload->metrics[size-1].has_is_historical = is_historical;
	if (is_historical) {
		payload->metrics[size-1].is_historical = is_historical;
	}
	payload->metrics[size-1].has_is_transient = is_transient;
	if (is_transient) {
		payload->metrics[size-1].is_transient = is_transient;
	}
	payload->metrics[size-1].has_is_null = is_null;
	if (is_null) {
		payload->metrics[size-1].is_null = is_null;
	}
	payload->metrics[size-1].has_metadata = false;
	payload->metrics[size-1].has_properties = false;

	// Default dynamically allocated members to NULL
	payload->metrics[size-1].value.string_value = NULL;

/* TODO
    bool has_metadata;
    com_cirruslink_sparkplug_protobuf_Payload_MetaData metadata;
    bool has_properties;
    com_cirruslink_sparkplug_protobuf_Payload_PropertySet properties;
*/

	printf("Setting datatype and value - value size is %zd\n", size_of_value);
	if (datatype == METRIC_DATA_TYPE_UNKNOWN) {
		printf("Can't create metric with unknown datatype!\n");
	} else if (datatype == METRIC_DATA_TYPE_INT8 || datatype == METRIC_DATA_TYPE_INT16 || datatype == METRIC_DATA_TYPE_INT32 ||
			datatype == METRIC_DATA_TYPE_UINT8 || datatype == METRIC_DATA_TYPE_UINT16 || datatype == METRIC_DATA_TYPE_UINT32) {
		printf("Setting datatype: %zd, with value: %d\n", datatype, *((uint32_t *)value));
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_int_value_tag;
		payload->metrics[size-1].value.int_value = *((uint32_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_INT64 || datatype == METRIC_DATA_TYPE_UINT64 || datatype == METRIC_DATA_TYPE_DATETIME) {
		printf("Setting datatype: %zd, with value: %zd\n", datatype, *((uint64_t *)value));
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_long_value_tag;
		payload->metrics[size-1].value.long_value = *((uint64_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_FLOAT) {
		printf("Setting datatype: %zd, with value: %f\n", datatype, *((float *)value));
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_float_value_tag;
		payload->metrics[size-1].value.float_value = *((float *)value);
	} else if (datatype == METRIC_DATA_TYPE_DOUBLE) {
		printf("Setting datatype: %zd, with value: %f\n", datatype, *((double *)value));
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_double_value_tag;
		payload->metrics[size-1].value.double_value = *((double *)value);
	} else if (datatype == METRIC_DATA_TYPE_BOOLEAN) {
		printf("Setting datatype: %zd, with value: %d\n", datatype, *((bool *)value));
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_boolean_value_tag;
		payload->metrics[size-1].value.boolean_value = *((bool *)value);
	} else if (datatype == METRIC_DATA_TYPE_STRING || datatype == METRIC_DATA_TYPE_TEXT || datatype == METRIC_DATA_TYPE_UUID) {
		printf("Setting datatype: %zd, with value: %s\n", datatype, (char *)value);
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_string_value_tag;
		payload->metrics[size-1].value.string_value = (char *)malloc(size_of_value*sizeof(char));
		strcpy(payload->metrics[size-1].value.string_value, (char *)value);
	} else if (datatype == METRIC_DATA_TYPE_BYTES) {
		printf("Setting datatype: %zd, with value ", datatype);
		int i;
		for (i = 0; i<size_of_value; i++) {
			if (i > 0) printf(":");
			printf("%02X", ((pb_byte_t *)value)[i]);
		}
		printf("\n");

/*
		pb_bytes_array_t *bytes_value = (pb_bytes_array_t *)malloc(sizeof(pb_bytes_array_t));
		bytes_value->size = size_of_value;
		memcpy(bytes_value->bytes, (pb_byte_t *)value, size_of_value);
		payload->metrics[size-1].which_value = com_cirruslink_sparkplug_protobuf_Payload_Metric_bytes_value_tag;
		payload->metrics[size-1].value.bytes_value = &bytes_value;
*/

	} else if (datatype == METRIC_DATA_TYPE_DATASET) {
		printf("Datatype DATASET - Not yet supported\n");
	} else if (datatype == METRIC_DATA_TYPE_FILE) {
		printf("Datatype FILE - Not yet supported\n");
	} else if (datatype == METRIC_DATA_TYPE_TEMPLATE) {
		printf("Datatype TEMPLATE - Not yet supported\n");
	} else {
		printf("Unknown datatype %ju\n", datatype);
	}

	payload->metrics_count++;
	printf("Size of metrics payload %d\n", payload->metrics_count);
}

void free_payload(com_cirruslink_sparkplug_protobuf_Payload *payload) {
	int i=0;
	for (i=0; i<payload->metrics_count; i++) {
		free(payload->metrics[i].name);

// More TODO...
	}
}

/***************************************************************************************************
 * Get current time in MS
 */
uint64_t get_current_timestamp() {
	// Set the timestamp
	struct timespec ts;
	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts.tv_sec = mts.tv_sec;
		ts.tv_nsec = mts.tv_nsec;
	#else
		clock_gettime(CLOCK_REALTIME, &ts);
	#endif
	return ts.tv_sec * UINT64_C(1000) + ts.tv_nsec / 1000000;
}

void print_payload(com_cirruslink_sparkplug_protobuf_Payload *payload) {

/*
        nbirth_payload.uuid = (char*)malloc((strlen("MyUUID")+1) * sizeof(char));
        strcpy(nbirth_payload.uuid, "MyUUID");
//      strcpy(nbirth_payload.body, "Setting the body to some chars");
*/
	printf("Payload:  has_timestamp: %s\n", payload->has_timestamp ? "true" : "false");
	if (payload->has_timestamp) {
		printf("Payload:  timestamp: %zd\n", payload->timestamp);
	}
	printf("Payload:  has_seq: %s\n", payload->has_seq ? "true" : "false");
	if (payload->has_seq) {
		printf("Payload:  seq: %zd\n", payload->seq);
	}
	printf("Payload:  UUID: %s\n", payload->uuid);

	printf("Payload:  Size of metric array: %d\n", payload->metrics_count);
	int i=0;
	for (i=0; i<payload->metrics_count; i++) {
		printf("Payload:  Metric %d:  name: %s\n", i, payload->metrics[i].name);
		printf("Payload:  Metric %d:  has_alias: %s\n", i, payload->metrics[i].has_alias ? "true" : "false");
		if (payload->metrics[i].has_alias) {
			printf("Payload:  Metric %d:  alias: %zd\n", i, payload->metrics[i].alias);
		}
		printf("Payload:  Metric %d:  has_timestamp: %s\n", i, payload->metrics[i].has_timestamp ? "true" : "false");
		if (payload->metrics[i].has_timestamp) {
			printf("Payload:  Metric %d:  timestamp: %zd\n", i, payload->metrics[i].timestamp);
		}
		printf("Payload:  Metric %d:  has_datatype: %s\n", i, payload->metrics[i].has_datatype ? "true" : "false");
		if (payload->metrics[i].has_datatype) {
			printf("Payload:  Metric %d:  datatype: %d\n", i, payload->metrics[i].datatype);
		}
		printf("Payload:  Metric %d:  has_is_historical: %s\n", i, payload->metrics[i].has_is_historical ? "true" : "false");
		if (payload->metrics[i].has_is_historical) {
			printf("Payload:  Metric %d:  is_historical: %s\n", i, payload->metrics[i].is_historical ? "true" : "false");
		}
		printf("Payload:  Metric %d:  has_is_transient: %s\n", i, payload->metrics[i].has_is_transient ? "true" : "false");
		if (payload->metrics[i].has_is_transient) {
			printf("Payload:  Metric %d:  is_transient: %s\n", i, payload->metrics[i].is_transient ? "true" : "false");
		}
		printf("Payload:  Metric %d:  has_is_null: %s\n", i, payload->metrics[i].has_is_null ? "true" : "false");
		if (payload->metrics[i].has_is_null) {
			printf("Payload:  Metric %d:  is_null: %s\n", i, payload->metrics[i].is_null ? "true" : "false");
		}
		printf("Payload:  Metric %d:  has_metadata: %s\n", i, payload->metrics[i].has_metadata ? "true" : "false");
		printf("Payload:  Metric %d:  has_properties: %s\n", i, payload->metrics[i].has_properties ? "true" : "false");

		if (payload->metrics[i].datatype == METRIC_DATA_TYPE_UNKNOWN) {
			printf("Payload:  Metric %d:  datatype: unknown datatype!\n", i);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_INT8 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_INT16 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_INT32 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_UINT8 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_UINT16 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_UINT32) {
			printf("Payload:  Metric %d:  datatype: %d, with value: %d\n", i, payload->metrics[i].datatype, payload->metrics[i].value.int_value);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_INT64 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_UINT64 ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_DATETIME) {
			printf("Payload:  Metric %d:  datatype: %d, with value: %zd\n", i, payload->metrics[i].datatype, payload->metrics[i].value.long_value);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_FLOAT) {
			printf("Payload:  Metric %d:  datatype: %d, with value: %f\n", i, payload->metrics[i].datatype, payload->metrics[i].value.float_value);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_DOUBLE) {
			printf("Payload:  Metric %d:  datatype: %d, with value: %f\n", i, payload->metrics[i].datatype, payload->metrics[i].value.double_value);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_BOOLEAN) {
			printf("Payload:  Metric %d:  datatype: %d, with value: %d\n", i, payload->metrics[i].datatype, payload->metrics[i].value.boolean_value);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_STRING ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_TEXT ||
					payload->metrics[i].datatype == METRIC_DATA_TYPE_UUID) {
			printf("Payload:  Metric %d:  datatype: %d, with value: %s\n", i, payload->metrics[i].datatype, payload->metrics[i].value.string_value);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_BYTES) {
			printf("Payload:  Metric %d:  datatype: %d, with value ", i, payload->metrics[i].datatype);
/*
			int i;
			for (i = 0; i<sizeof(payload->metrics[i].value.bytes_value); i++) {
				if (i > 0) printf(":");
				printf("%02X", payload->metrics[i].value.bytes_value[i]);
			}
*/
			printf("\n");
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_DATASET) {
			printf("Payload:  Metric %d:  datatype DATASET - Not yet supported\n", i);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_FILE) {
			printf("Payload:  Metric %d:  datatype FILE - Not yet supported\n", i);
		} else if (payload->metrics[i].datatype == METRIC_DATA_TYPE_TEMPLATE) {
			printf("Payload:  Metric %d:  datatype TEMPLATE - Not yet supported\n", i);
		} else {
			printf("Payload:  Metric %d:  datatype: %d\n", i, payload->metrics[i].datatype);
		}
	}
}


size_t encode_payload(uint8_t **buffer, size_t buffer_length, com_cirruslink_sparkplug_protobuf_Payload *payload) {
        size_t message_length;
	bool node_status;

	// Create the stream
	pb_ostream_t node_stream = pb_ostream_from_buffer(*buffer, buffer_length);

	// Encode the payload
	printf("Encoding...\n");
	node_status = pb_encode(&node_stream, com_cirruslink_sparkplug_protobuf_Payload_fields, payload);
	message_length = node_stream.bytes_written;
	printf("Message length: %zd\n", message_length);

        // Error Check
        if (!node_status) {
                printf("Encoding failed: %s\n", PB_GET_ERROR(&node_stream));
                return -1;
        } else {
                printf("Encoding succeeded\n");
		return message_length;
        }
}

void get_next_payload(com_cirruslink_sparkplug_protobuf_Payload *payload) {
	// Initialize payload
	printf("Current Sequence Number: %zd\n", seq);
	payload->has_timestamp = true;
	payload->timestamp = get_current_timestamp();
	payload->metrics_count = 0;
	payload->metrics = NULL;
	payload->has_seq = true;
	payload->seq = seq;
	payload->uuid = NULL;
	payload->body = NULL;
	payload->extensions = NULL;

	// Increment/wrap the sequence number
	if (seq == 256) {
		seq = 0;
	} else {
		seq++;
	}
}












void decode_metric() {
}

bool decode_payload(com_cirruslink_sparkplug_protobuf_Payload *payload, const void *binary_payload, int binary_payloadlen) {

	pb_istream_t stream = pb_istream_from_buffer(binary_payload, binary_payloadlen);

	printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
	printf("Bytes Remaining: %zd\n", stream.bytes_left);

	bool status;

	pb_wire_type_t payload_wire_type;
	uint32_t payload_tag;
	bool payload_eof;
	const pb_field_t *payload_field;

	while (pb_decode_tag(&stream, &payload_wire_type, &payload_tag, &payload_eof)) {
		printf("payload_eof: %s\n", payload_eof ? "true" : "false");
		printf("\tBytes Remaining: %zd\n", stream.bytes_left);
		printf("\tWiretype: %d\n", payload_wire_type);
		printf("\tTag: %d\n", payload_tag);

		if (payload_wire_type == PB_WT_VARINT) {
			for (payload_field = com_cirruslink_sparkplug_protobuf_Payload_fields; payload_field->tag != 0; payload_field++) {
				if (payload_field->tag == payload_tag && (((payload_field->type & PB_LTYPE_VARINT) == PB_LTYPE_VARINT) ||
										((payload_field->type & PB_LTYPE_UVARINT) == PB_LTYPE_UVARINT))) {
					printf("\tWire type is PB_WT_VARINT\n");
					uint64_t dest;
					status = pb_decode_varint(&stream, &dest);
					if (status) {
						printf("\tVARINT - Success - new value: %ld\n", dest);
					} else {
						printf("\tVARINT - Failed to decode variant!\n");
					}
				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_SVARINT) == PB_LTYPE_SVARINT)) {
					printf("\tWire type is PB_WT_SVARINT\n");
					int64_t dest;
					status = pb_decode_svarint(&stream, &dest);
					if (status) {
						printf("\tVARINT - Success - new value: %ld\n", dest);
					} else {
						printf("\tVARINT - Failed to decode variant!\n");
					}
				}
			}
		} else if (payload_wire_type == PB_WT_64BIT) {
			printf("\tWire type is PB_WT_64BIT\n");
		} else if (payload_wire_type == PB_WT_STRING) {
			printf("\tWire type is PB_WT_STRING\n");
			for (payload_field = com_cirruslink_sparkplug_protobuf_Payload_fields; payload_field->tag != 0; payload_field++) {
				if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_SUBMESSAGE) == PB_LTYPE_SUBMESSAGE)) {
					printf("\tFound a PB_LTYPE_SUBMESSAGE\n");

					// Found a metric - get the ptr to it
					com_cirruslink_sparkplug_protobuf_Payload_Metric metric = com_cirruslink_sparkplug_protobuf_Payload_Metric_init_zero;
					//printf("\t\tsizeof(field->ptr): %zd\n", sizeof(field->ptr));

					pb_istream_t substream;
					const pb_field_t *submsg_fields = (const pb_field_t *)payload_field->ptr;
					if (!pb_make_string_substream(&stream, &substream)) {
						printf("FAILED\n");
					}

					if (payload_field->ptr == NULL) {
						printf("invalid field descriptor\n");
					}

					pb_wire_type_t metric_wire_type;
					uint32_t metric_tag;
					bool metric_eof;
					const pb_field_t *metric_field;

					while (pb_decode_tag(&substream, &metric_wire_type, &metric_tag, &metric_eof)) {
						printf("\teof: %s\n", metric_eof ? "true" : "false");
						printf("\t\tBytes Remaining: %zd\n", substream.bytes_left);
						printf("\t\tWiretype: %d\n", metric_wire_type);
						printf("\t\tTag: %d\n", metric_tag);

						if (metric_wire_type == PB_WT_VARINT) {
							printf("\t\tMetric Wire type is PB_WT_VARINT\n");
							for (metric_field = com_cirruslink_sparkplug_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
								if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_VARINT) == PB_LTYPE_VARINT) ||
													((metric_field->type & PB_LTYPE_UVARINT) == PB_LTYPE_UVARINT))) {
									printf("\t\tWire type is PB_WT_VARINT\n");
									uint64_t dest;
									status = pb_decode_varint(&substream, &dest);
									if (status) {
										printf("\t\tVARINT - Success - new value: %ld\n", dest);
									} else {
										printf("\t\tVARINT - Failed to decode variant!\n");
									}
								} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_SVARINT) == PB_LTYPE_SVARINT)) {
									printf("\t\tWire type is PB_WT_SVARINT\n");
									int64_t dest;
									status = pb_decode_svarint(&substream, &dest);
									if (status) {
										printf("\t\tVARINT - Success - new value: %ld\n", dest);
									} else {
										printf("\t\tVARINT - Failed to decode variant!\n");
									}
								}
							}
						} else if (metric_wire_type == PB_WT_64BIT) {
							printf("\t\tMetric Wire type is PB_WT_64BIT\n");
						} else if (metric_wire_type == PB_WT_STRING) {
							printf("\t\tMetric Wire type is PB_WT_STRING\n");

							for (metric_field = com_cirruslink_sparkplug_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
								if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_SUBMESSAGE) == PB_LTYPE_SUBMESSAGE)) {
									printf("\t\tFound a PB_LTYPE_SUBMESSAGE\n");
								} else if (metric_field->tag == metric_tag &&
											((metric_field->type & PB_LTYPE_FIXED_LENGTH_BYTES) == PB_LTYPE_FIXED_LENGTH_BYTES)) {
									printf("\t\tFound a PB_LTYPE_FIXED_LENGTH_BYTES\n");
								} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_STRING) == PB_LTYPE_STRING)) {
									printf("\t\tFound a PB_LTYPE_STRING\n");

									// Get the string size
									pb_byte_t string_size[1];
									status = pb_read(&substream, string_size, 1);
									if (status) {
										printf("\t\tString Size: %d\n", string_size[0]);
									}

									pb_byte_t dest[string_size[0]+1];
									status = pb_read(&substream, dest, string_size[0]);
									if (status) {
										printf("\t\tRead the String! %s\n", dest);
									} else {
										printf("\t\tFailed to read the String...\n");
									}


								} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_BYTES) == PB_LTYPE_BYTES)) {
									printf("\t\tFound a PB_LTYPE_BYTES\n");
								} else {
									printf("\t\tother: %d\n", metric_field->type);
								}
							}

						} else if (metric_wire_type == PB_WT_32BIT) {
							printf("\t\tMetric Wire type is PB_WT_32BIT\n");
						} else {
							printf("\t\tMetric Other? %d\n", metric_wire_type);
						}
					}

					// Close the substream
					pb_close_string_substream(&stream, &substream);

				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_FIXED_LENGTH_BYTES) == PB_LTYPE_FIXED_LENGTH_BYTES)) {
					printf("\tFound a PB_LTYPE_FIXED_LENGTH_BYTES\n");
				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_STRING) == PB_LTYPE_STRING)) {
					printf("\tFound a PB_LTYPE_STRING\n");
				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_BYTES) == PB_LTYPE_BYTES)) {
					printf("\tFound a PB_LTYPE_BYTES\n");
				} else {
					printf("\tother: %d\n", payload_field->type);
				}
			}
		} else if (payload_wire_type == PB_WT_32BIT) {
			printf("\tWire type is PB_WT_32BIT\n");
		} else {
			printf("\tUnknown wiretype...\n");
		}
	}

	// Print the message data
	//print_payload(&inbound_payload);
	return true;
}