#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include "netdb.h"
#include <finsh.h>
#include <webclient.h>
#include "string.h"

#include <dw1000_usr.h>
#include <cJSON.h>

#define DEBUG_UDP_CLIENT

#define DBG_TAG "UDP"
#ifdef DEBUG_UDP_CLIENT
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO /* DBG_ERROR */
#endif
#include <rtdbg.h>
// static char url[256];
// static int port = 8080;
const char send_data[] = "This is UDP Client from lch test.\n"; /* 发送用到的数据 */

#if 0
//lch add
#define UDP_SERVER_PORT 3066
#define UDP_SERVER_IP "192.168.1.12"
//test
#else
#define UDP_SERVER_PORT 3005
#define UDP_SERVER_IP "192.168.1.8"
#endif
//lch add end
static void udpclient(void *arg)
{
	message_mdps_t msg;
	struct hostent *host;
	static int sock;
	struct sockaddr_in server_addr;

#if 0
    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = (struct hostent *) gethostbyname(url);
    if (host == RT_NULL)
    {
        LOG_E("Get host by name failed!");
        return;
    }
#endif

	/* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		LOG_E("Create socket error");
		return;
	}

	host = (struct hostent *)gethostbyname(UDP_SERVER_IP);
	/* 初始化预连接的服务端地址 */
	server_addr.sin_family = AF_INET;
	//server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	server_addr.sin_port = htons(UDP_SERVER_PORT);
	// server_addr.sin_addr.s_addr = inet_addr(UDP_SERVER_IP);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
	while (1)
	{
//test
#if 0
        /* 发送数据到服务远端 */
        sendto(sock, send_data, rt_strlen(send_data), 0,
               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
		
        /* 线程休眠一段时间 */
        rt_thread_mdelay(1000);
		rt_kprintf("udp client send message\r\n");
#else
		if (rt_mq_recv(&message_send, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
		{
			sendto(sock, msg.buf, msg.len, 0,
				   (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
			// rt_kprintf("%s\r\n", msg.buf);
		}
#endif
	}
}

//--------------------------------------------------------
static void start_udp_client(int argc, char **argv)
{
	rt_thread_t tid;
	tid = rt_thread_create("udp_client",
						   udpclient, RT_NULL,
						   2048, RT_THREAD_PRIORITY_MAX / 3, 20);
	if (tid != RT_NULL)
	{
		rt_thread_startup(tid);
	}
	return;
}

//-------------------------------------------------------

#define GET_HEADER_BUFSZ 1024
#define GET_REV_BUF_SIZE 1024
static int send_http_get(const char *uri, unsigned char *buf, int *len)
{

	int ret = 0, resp_status;
	struct webclient_session *session = RT_NULL;
	int file_size = 0, length, total_length = 0;
	rt_uint8_t *buffer_read = RT_NULL;

	/* create webclient session and set header response size */
	session = webclient_session_create(GET_HEADER_BUFSZ);
	if (!session)
	{
		LOG_E("open uri failed.");
		ret = -RT_ERROR;
		goto __exit;
	}

	/* send GET request by default header */
	if ((resp_status = webclient_get(session, uri)) != 200)
	{
		LOG_E("webclient GET request failed, response(%d) error.", resp_status);
		ret = -RT_ERROR;
		goto __exit;
	}
	file_size = webclient_content_length_get(session);
	rt_kprintf("http file_size:%d\n", file_size);

	if (file_size == 0)
	{
		LOG_E("Request file size is 0!");
		ret = -RT_ERROR;
		goto __exit;
	}
	else if (file_size < 0)
	{
		LOG_E("webclient GET request type is chunked.");
		ret = -RT_ERROR;
		goto __exit;
	}
	buffer_read = web_malloc(GET_REV_BUF_SIZE);
	if (buffer_read == RT_NULL)
	{
		LOG_E("No memory for http get rev!");
		ret = -RT_ERROR;
		goto __exit;
	}
	memset(buffer_read, 0x00, GET_REV_BUF_SIZE);
	do
	{
		length = webclient_read(session, buffer_read, file_size - total_length > GET_REV_BUF_SIZE ? GET_REV_BUF_SIZE : file_size - total_length);
		if (length > 0)
		{
			memcpy(buf + total_length, buffer_read, length);
			total_length += length;
			*len = total_length;
			LOG_I("get length:(%d)!", length);
		}
		else
		{
			LOG_E("Exit: server return err (%d)!", length);
			ret = -RT_ERROR;
			goto __exit;
		}

	} while (total_length != file_size);

	ret = RT_EOK;

	if (total_length == file_size)
	{
		LOG_I("get data ok!\r\n");
		//memcpy(buf,buffer_read,total_length);
		if (session != RT_NULL)
			webclient_close(session);
		if (buffer_read != RT_NULL)
			web_free(buffer_read);
	}
	return ret;

__exit:
	if (session != RT_NULL)
		webclient_close(session);
	if (buffer_read != RT_NULL)
		web_free(buffer_read);
	return ret;
}
//-----------------------------------------------------
const char *test_uri = "http://192.168.1.12:8066/netlog?data=";

void anchor_tick_str(char *pdst);
void cmd_return_str(char *pdst);
void cmd_devid_str(char *pdst);

char buf[512];
//test
char url_tick[512];
char url_cmd_return[256];
char url_devid[256];
char data[512];

// void send_http_get_test(void *p)
// {
// 	int cnt = 0;
// 	int ret = 0;
// 	int len;
// 	anchor_tick_str(data);
// 	sprintf(url_tick, "%s%s", test_uri, data);
// 	cmd_return_str(data);
// 	sprintf(url_cmd_return, "%s%s", test_uri, data);
// 	cmd_devid_str(data);
// 	sprintf(url_devid, "%s%s", test_uri, data);

// 	//rt_kprintf("url%s\r\n",url);
// 	//rt_kprintf("url");
// 	while (1)
// 	{

// 		switch (cnt)
// 		{
// 		case 0:
// 			ret = send_http_get(url_tick, buf, &len);
// 			cnt++;
// 			break;
// 		case 1:
// 			ret = send_http_get(url_cmd_return, buf, &len);
// 			cnt++;
// 			break;
// 		case 2:
// 			ret = send_http_get(url_devid, buf, &len);
// 			cnt++;
// 			break;
// 		default:
// 			break;
// 		}
// 		if (ret == RT_EOK)
// 		{
// 			rt_kprintf("get data:%s", buf);
// 		}
// 		if (cnt >= 3)
// 			cnt = 0;

// 		rt_thread_mdelay(1000);
// 	}
// }

void udpclient_test(int argc, char **argv)
{
	int sock, port, count;
	struct hostent *host;
	struct sockaddr_in server_addr;
	const char *url;
	message_mdps_t msg;

	/* 接收到的参数小于 3 个 */
	if (argc < 3)
	{
		rt_kprintf("Usage: udpclient URL PORT [COUNT = 10]\n");
		rt_kprintf("Like: tcpclient 192.168.12.44 5000\n");
		return;
	}

	url = argv[1];
	port = strtoul(argv[2], 0, 10);

	if (argc > 3)
		count = strtoul(argv[3], 0, 10);
	else
		count = 10;

	/* 通过函数入口参数 url 获得 host 地址（如果是域名，会做域名解析） */
	host = (struct hostent *)gethostbyname(url);

	/* 创建一个 socket，类型是 SOCK_DGRAM，UDP 类型 */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		rt_kprintf("Socket error\n");
		return;
	}

	/* 初始化预连接的服务端地址 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	/* 总计发送 count 次数据 */
	while (count)
	{
		// /* 发送数据到服务远端 */
		// sendto(sock, send_data, strlen(send_data), 0,
		// 	   (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

		// /* 线程休眠一段时间 */
		// rt_thread_delay(50);

		// /* 计数值减一 */
		// count--;

		if (rt_mq_recv(&message_send, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
		{
			sendto(sock, msg.buf, msg.len, 0,
				   (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
			// rt_kprintf("%s\r\n", msg.buf);
		}
	}

	/* 关闭这个 socket */
	closesocket(sock);
}

MSH_CMD_EXPORT(udpclient_test, Start a udp test);
MSH_CMD_EXPORT(start_udp_client, Start a udp test);

// MSH_CMD_EXPORT(send_http_get_test, test send http get);

//MSH_CMD_EXPORT(test_json, json test);
