/*
 C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h> //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h> // for close
#include <stdlib.h> // pulls in declaration of malloc, free
#include "ua_transport_generated.h"
#include "ua_namespace_0.h"

UA_Int32 sendHello(UA_Int32 sock, UA_String *endpointURL) {
	UA_ByteString *message;
	UA_ByteString_new(&message);
	UA_ByteString_newMembers(message, 1000);

	UA_UInt32 offset = 0;
	UA_String endpointUrl;
	UA_String_copy(endpointURL, &endpointUrl);

	UA_TcpMessageHeader messageHeader;
	UA_TcpHelloMessage hello;
	messageHeader.isFinal = 'F';
	messageHeader.messageType = UA_MESSAGETYPE_HEL;

	hello.endpointUrl = endpointUrl;
	hello.maxChunkCount = 1;
	hello.maxMessageSize = 16777216;
	hello.protocolVersion = 0;
	hello.receiveBufferSize = 65536;
	hello.sendBufferSize = 65536;

	messageHeader.messageSize = UA_TcpHelloMessage_calcSizeBinary(
			(UA_TcpHelloMessage const*) &hello)
			+ UA_TcpMessageHeader_calcSizeBinary(
					(UA_TcpMessageHeader const*) &messageHeader);

	UA_TcpMessageHeader_encodeBinary(
			(UA_TcpMessageHeader const*) &messageHeader, message, &offset);
	UA_TcpHelloMessage_encodeBinary((UA_TcpHelloMessage const*) &hello, message,
			&offset);

	UA_Int32 sendret = send(sock, message->data, offset, 0);

	UA_ByteString_delete(message);

	free(endpointUrl.data);
	if (sendret < 0) {
		return 1;
	}
	return 0;

}
int sendOpenSecureChannel(UA_Int32 sock) {
	UA_ByteString *message;
	UA_ByteString_new(&message);
	UA_ByteString_newMembers(message, 1000);

	UA_UInt32 offset = 0;

	UA_TcpMessageHeader msghdr;
	msghdr.isFinal = 'F';
	msghdr.messageType = UA_MESSAGETYPE_OPN;
	msghdr.messageSize = 135;

	UA_TcpMessageHeader_encodeBinary(&msghdr, message, &offset);
	UA_UInt32 secureChannelId = 0;
	UA_UInt32_encodeBinary(&secureChannelId, message, &offset);
	UA_String securityPolicy;

	UA_String_copycstring("http://opcfoundation.org/UA/SecurityPolicy#None",
			&securityPolicy);
	UA_String_encodeBinary(&securityPolicy, message, &offset);

	UA_String senderCert;
	senderCert.data = UA_NULL;
	senderCert.length = -1;
	UA_String_encodeBinary(&senderCert, message, &offset);

	UA_String receiverCertThumb;
	receiverCertThumb.data = UA_NULL;
	receiverCertThumb.length = -1;
	UA_String_encodeBinary(&receiverCertThumb, message, &offset);

	UA_UInt32 sequenceNumber = 51;
	UA_UInt32_encodeBinary(&sequenceNumber, message, &offset);

	UA_UInt32 requestId = 1;
	UA_UInt32_encodeBinary(&requestId, message, &offset);

	UA_NodeId type;
	type.identifier.numeric = 446;
	type.identifierType = UA_NODEIDTYPE_NUMERIC;
	type.namespaceIndex = 0;
	UA_NodeId_encodeBinary(&type, message, &offset);

	UA_OpenSecureChannelRequest opnSecRq;
	UA_OpenSecureChannelRequest_init(&opnSecRq);

	opnSecRq.requestHeader.timestamp = UA_DateTime_now();

	UA_ByteString_newMembers(&opnSecRq.clientNonce, 1);
	opnSecRq.clientNonce.data[0] = 0;

	opnSecRq.clientProtocolVersion = 0;
	opnSecRq.requestedLifetime = 30000;
	opnSecRq.securityMode = UA_SECURITYMODE_NONE;
	opnSecRq.requestType = UA_SECURITYTOKENREQUESTTYPE_ISSUE;

	opnSecRq.requestHeader.authenticationToken.identifier.numeric = 10;
	opnSecRq.requestHeader.authenticationToken.identifierType =
			UA_NODEIDTYPE_NUMERIC;
	opnSecRq.requestHeader.authenticationToken.namespaceIndex = 10;

	UA_OpenSecureChannelRequest_encodeBinary(&opnSecRq, message, &offset);
	UA_Int32 sendret = send(sock, message->data, offset, 0);
	UA_ByteString_delete(message);
	free(securityPolicy.data);
	if (sendret < 0) {
		printf("send opensecurechannel failed");
		return 1;
	}
	return 0;
}

UA_Int32 sendCreateSession(UA_Int32 sock, UA_UInt32 channelId,
		UA_UInt32 tokenId, UA_UInt32 sequenceNumber, UA_UInt32 requestId,
		UA_String *endpointUrl) {
	UA_ByteString *message;
	UA_ByteString_new(&message);
	UA_ByteString_newMembers(message, 65536);
	UA_UInt32 tmpChannelId = channelId;
	UA_UInt32 offset = 0;

	UA_TcpMessageHeader msghdr;
	msghdr.isFinal = 'F';
	msghdr.messageType = UA_MESSAGETYPE_MSG;
	msghdr.messageSize = 164;

	UA_TcpMessageHeader_encodeBinary(&msghdr, message, &offset);

	UA_UInt32_encodeBinary(&tmpChannelId, message, &offset);
	UA_UInt32_encodeBinary(&tokenId, message, &offset);
	UA_UInt32_encodeBinary(&sequenceNumber, message, &offset);
	UA_UInt32_encodeBinary(&requestId, message, &offset);

	UA_NodeId type;
	type.identifier.numeric = 461;
	type.identifierType = UA_NODEIDTYPE_NUMERIC;
	type.namespaceIndex = 0;
	UA_NodeId_encodeBinary(&type, message, &offset);

	UA_CreateSessionRequest rq;
	UA_RequestHeader_init(&rq.requestHeader);

	rq.requestHeader.requestHandle = 1;
	rq.requestHeader.timestamp = UA_DateTime_now();
	rq.requestHeader.timeoutHint = 10000;
	rq.requestHeader.auditEntryId.length = -1;
	rq.requestHeader.authenticationToken.identifier.numeric = 10;
	rq.requestHeader.authenticationToken.identifierType = UA_NODEIDTYPE_NUMERIC;
	rq.requestHeader.authenticationToken.namespaceIndex = 10;
	UA_String_copy(endpointUrl, &rq.endpointUrl);
	rq.clientDescription.applicationName.locale.length = -1;
	rq.clientDescription.applicationName.text.length = -1;

	rq.clientDescription.applicationUri.length = -1;
	rq.clientDescription.discoveryProfileUri.length = -1;
	rq.clientDescription.discoveryUrls = UA_NULL;
	rq.clientDescription.discoveryUrlsSize = -1;
	rq.clientDescription.gatewayServerUri.length = -1;
	rq.clientDescription.productUri.length = -1;

	UA_String_copycstring("mysession", &rq.sessionName);

	UA_String_copycstring("abcd", &rq.clientCertificate);

	UA_ByteString_newMembers(&rq.clientNonce, 1);
	rq.clientNonce.data[0] = 0;

	rq.requestedSessionTimeout = 1200000;
	rq.maxResponseMessageSize = UA_INT32_MAX;

	UA_CreateSessionRequest_encodeBinary(&rq, message, &offset);

	UA_Int32 sendret = send(sock, message->data, offset, 0);
	UA_ByteString_delete(message);
	free(rq.sessionName.data);
	free(rq.clientCertificate.data);
	if (sendret < 0) {
		printf("send opensecurechannel failed");
		return 1;
	}
	return 0;
}
UA_Int32 sendActivateSession(UA_Int32 sock, UA_UInt32 channelId,
		UA_UInt32 tokenId, UA_UInt32 sequenceNumber, UA_UInt32 requestId) {
	UA_ByteString *message;
	UA_ByteString_new(&message);
	UA_ByteString_newMembers(message, 65536);
	UA_UInt32 tmpChannelId = channelId;
	UA_UInt32 offset = 0;

	UA_TcpMessageHeader msghdr;
	msghdr.isFinal = 'F';
	msghdr.messageType = UA_MESSAGETYPE_MSG;
	msghdr.messageSize = 86;

	UA_TcpMessageHeader_encodeBinary(&msghdr, message, &offset);

	UA_UInt32_encodeBinary(&tmpChannelId, message, &offset);
	UA_UInt32_encodeBinary(&tokenId, message, &offset);
	UA_UInt32_encodeBinary(&sequenceNumber, message, &offset);
	UA_UInt32_encodeBinary(&requestId, message, &offset);

	UA_NodeId type;
	type.identifier.numeric = 467;
	type.identifierType = UA_NODEIDTYPE_NUMERIC;
	type.namespaceIndex = 0;
	UA_NodeId_encodeBinary(&type, message, &offset);

	UA_ActivateSessionRequest rq;
	UA_ActivateSessionRequest_init(&rq);
	rq.requestHeader.requestHandle = 2;

	rq.requestHeader.authenticationToken.identifier.numeric = 10;
	rq.requestHeader.authenticationToken.identifierType = UA_NODEIDTYPE_NUMERIC;
	rq.requestHeader.authenticationToken.namespaceIndex = 10;

	UA_ActivateSessionRequest_encodeBinary(&rq, message, &offset);
	UA_Int32 sendret = send(sock, message->data, offset, 0);
	UA_ByteString_delete(message);

	if (sendret < 0) {
		printf("send opensecurechannel failed");
		return 1;
	}
	return 0;

}

UA_Int32 sendReadRequest(UA_Int32 sock, UA_UInt32 channelId, UA_UInt32 tokenId,
		UA_UInt32 sequenceNumber, UA_UInt32 requestId) {
	UA_ByteString *message;
	UA_ByteString_new(&message);
	UA_ByteString_newMembers(message, 65536);
	UA_UInt32 tmpChannelId = channelId;
	UA_UInt32 offset = 0;

	UA_TcpMessageHeader msghdr;
	msghdr.isFinal = 'F';
	msghdr.messageType = UA_MESSAGETYPE_MSG;
	msghdr.messageSize = 91;

	UA_TcpMessageHeader_encodeBinary(&msghdr, message, &offset);

	UA_UInt32_encodeBinary(&tmpChannelId, message, &offset);
	UA_UInt32_encodeBinary(&tokenId, message, &offset);
	UA_UInt32_encodeBinary(&sequenceNumber, message, &offset);
	UA_UInt32_encodeBinary(&requestId, message, &offset);

	UA_NodeId type;
	type.identifier.numeric = 631;
	type.identifierType = UA_NODEIDTYPE_NUMERIC;
	type.namespaceIndex = 0;
	UA_NodeId_encodeBinary(&type, message, &offset);

	UA_ReadRequest rq;
	UA_ReadRequest_init(&rq);

	rq.maxAge = 0;

	UA_Array_new((void **) &rq.nodesToRead, 1, &UA_[UA_READVALUEID]);
	rq.nodesToReadSize = 1;
	UA_ReadValueId_init(&(rq.nodesToRead[0]));
	rq.nodesToRead[0].attributeId = 13; //UA_ATTRIBUTEID_VALUE
	UA_NodeId_init(&(rq.nodesToRead[0].nodeId));
	rq.nodesToRead[0].nodeId.identifierType = UA_NODEIDTYPE_NUMERIC;
	rq.nodesToRead[0].nodeId.identifier.numeric = 2255;
	UA_QualifiedName_init(&(rq.nodesToRead[0].dataEncoding));
	rq.requestHeader.timeoutHint = 10000;
	rq.requestHeader.timestamp = UA_DateTime_now();
	rq.timestampsToReturn = 0x03;
	UA_ReadRequest_encodeBinary(&rq, message, &offset);
	UA_Int32 sendret = send(sock, message->data, offset, 0);
	UA_ByteString_delete(message);

	rq.requestHeader.authenticationToken.identifier.numeric = 10;
	rq.requestHeader.authenticationToken.identifierType = UA_NODEIDTYPE_NUMERIC;
	rq.requestHeader.authenticationToken.namespaceIndex = 10;

	rq.requestHeader.requestHandle = 3;
	if (sendret < 0) {
		printf("send opensecurechannel failed");
		return 1;
	}
	return 0;
}
int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in server;

	UA_ByteString *reply;
	UA_ByteString_new(&reply);
	UA_ByteString_newMembers(reply, 65536);

//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket");
	}
	server.sin_addr.s_addr = inet_addr("134.130.125.98");
	server.sin_family = AF_INET;
	server.sin_port = htons(16664);
//Connect to remote server
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("connect failed. Error");
		return 1;
	}
	UA_String *endpointUrl;
	UA_String_new(&endpointUrl);

	UA_String_copycstring("opc.tcp://134.130.125.48:16663", endpointUrl);
	sendHello(sock, endpointUrl);
	int received = recv(sock, reply->data, reply->length, 0);
	sendOpenSecureChannel(sock);
	received = recv(sock, reply->data, reply->length, 0);

	UA_UInt32 recvOffset = 0;
	UA_TcpMessageHeader msghdr;
	UA_TcpMessageHeader_decodeBinary(reply, &recvOffset, &msghdr);
	UA_UInt32 secureChannelId;
	UA_UInt32_decodeBinary(reply, &recvOffset, &secureChannelId);

	sendCreateSession(sock, secureChannelId, 1, 52, 2, endpointUrl);
	received = recv(sock, reply->data, reply->length, 0);
	sendActivateSession(sock, secureChannelId, 1, 53, 3);
	received = recv(sock, reply->data, reply->length, 0);
	UA_DateTime tic, toc;
	for (UA_Int32 i = 0; i < 100; i++) {
		tic = UA_DateTime_now();
		sendReadRequest(sock, secureChannelId, 1+i, 54+i, 4+i);
		toc = UA_DateTime_now() - tic;
		received = recv(sock, reply->data, 2000, 0);
		UA_Int64 diff = ((UA_Int64) toc );
		printf("read req took: %llu  \n", diff);
	}

	/*
	 UA_TcpMessageHeader reqTcpHeader;
	 UA_UInt32 reqSecureChannelId = 0;
	 UA_UInt32 reqTokenId = 0;
	 UA_SequenceHeader reqSequenceHeader;
	 UA_NodeId reqRequestType;
	 UA_ReadRequest req;
	 UA_RequestHeader reqHeader;
	 UA_NodeId reqHeaderAuthToken;
	 UA_ExtensionObject reqHeaderAdditionalHeader;
	 UA_NodeId_init(&reqRequestType);
	 reqRequestType.identifierType = UA_NODEIDTYPE_NUMERIC;
	 reqRequestType.identifier.numeric = 631; //read request
	 UA_ReadRequest_init(&req);
	 req.requestHeader = reqHeader;
	 UA_RequestHeader_init(&(req.requestHeader));
	 req.requestHeader.authenticationToken = reqHeaderAuthToken;
	 UA_NodeId_init(&(req.requestHeader.authenticationToken));
	 req.requestHeader.additionalHeader = reqHeaderAdditionalHeader;
	 UA_ExtensionObject_init(&(req.requestHeader.additionalHeader));
	 UA_Array_new((void **)&req.nodesToRead, 1, &UA_.types[UA_READVALUEID]);
	 req.nodesToReadSize = 1;
	 UA_ReadValueId_init(&(req.nodesToRead[0]));
	 req.nodesToRead[0].attributeId = 13; //UA_ATTRIBUTEID_VALUE
	 UA_NodeId_init(&(req.nodesToRead[0].nodeId));
	 req.nodesToRead[0].nodeId.identifierType = UA_NODEIDTYPE_NUMERIC;
	 req.nodesToRead[0].nodeId.identifier.numeric = 2255;
	 UA_QualifiedName_init(&(req.nodesToRead[0].dataEncoding));
	 messageEncodedLength = UA_TcpMessageHeader_calcSizeBinary(&reqTcpHeader) +
	 UA_UInt32_calcSizeBinary(&reqSecureChannelId)+
	 UA_UInt32_calcSizeBinary(&reqTokenId)+
	 UA_SequenceHeader_calcSizeBinary(&reqSequenceHeader)+
	 UA_NodeId_calcSizeBinary(&reqRequestType) +
	 UA_ReadRequest_calcSizeBinary(&req);
	 UA_TcpMessageHeader_init(&reqTcpHeader);
	 reqTcpHeader.messageType = UA_MESSAGETYPE_MSG;
	 reqTcpHeader.messageSize = messageEncodedLength;
	 reqTcpHeader.isFinal = 'F';
	 UA_TcpMessageHeader_encodeBinary(&reqTcpHeader, &message, &messagepos);
	 UA_UInt32_encodeBinary(&reqSecureChannelId, &message, &messagepos);
	 UA_UInt32_encodeBinary(&reqTokenId, &message, &messagepos);
	 UA_SequenceHeader_encodeBinary(&reqSequenceHeader, &message, &messagepos);
	 UA_NodeId_encodeBinary(&reqRequestType, &message, &messagepos);
	 UA_ReadRequest_encodeBinary(&req, &message, &messagepos);
	 */
//Send some data
//Receive a reply from the server
	if (received < 0) {
		puts("recv failed");
		return 1;
	}
	for (int i = 0; i < received; i++) {
//show only printable ascii
		if (reply->data[i] >= 32 && reply->data[i] <= 126)
			printf("%c", reply->data[i]);
	}
	printf("\n");

	close(sock);
	return 0;
}
