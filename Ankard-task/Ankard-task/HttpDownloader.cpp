#include "HttpDownloader.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "WinsockUtils.h"
#include <fstream>
#include <sstream>
#include <regex>

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace task;


// ----------------------------------------------------------------------------
// ������� ������� �� ��������� �����.
// ----------------------------------------------------------------------------
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName);

// ----------------------------------------------------------------------------
// ��������� ����������� ����� ���� � ������� ��� � �����, ���� �� ��������� ������� �����.
// ----------------------------------------------------------------------------
void ReceiveResponse(SOCKET connection, std::ostream& out);



// ----------------------------------------------------------------------------
void HttpDownloader::DownloadFile(std::ostream& out, std::string_view pageUrl)
{
	// �������������� ����� (TCP).
	SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
		throw WinsockSocketException();

	try
	{
		// �������� ��� �����.
		auto hostName = GetHttpHostNameByUrl(pageUrl);

		// ��������� ����� �������.
		sockaddr_in server;
		ZeroMemory(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = GetServerAddress(hostName);
		server.sin_port = htons(80);

		// ������������� ����������.
		if (bool failed = connect(connection, (sockaddr*)& server, sizeof(server)); failed)
			throw WinsockSocketException();

		// ��������� ���� ��������.
		SendRequest(connection, pageUrl, hostName);
		ReceiveResponse(connection, out);
		out.flush();

		closesocket(connection);
	}
	catch (...)
	{
		closesocket(connection);
		throw;
	}
}


// ----------------------------------------------------------------------------
void HttpDownloader::DownloadPageWithDependencies(std::string_view difectory, std::string_view fileName, std::string_view pageUrl)
{
	using std::string;

	// ���������� ����� �� ����� ����� � ����� ��� ����.
	std::ostringstream ssTmp;
	ssTmp << difectory << '/' << fileName;
	string fullSubirectoryName = ssTmp.str();	// ������ ��� ����� ���� ����� ������������ �����������.

	ssTmp << ".html";
	string fullFilenameName = ssTmp.str();	// ������ ��� html-����� �� �����.

	// �������� ��� html-��������.
	ssTmp.clear();
	DownloadFile(ssTmp, pageUrl);
	string source = ssTmp.str();
	
	// ��������� html-����.
	std::ofstream file(fullFilenameName);
	file << source;
	file.flush();

	// TODO: ����������� � ��������� �����������
}


// ----------------------------------------------------------------------------
void SendRequest(SOCKET connection, std::string_view pageUrl, std::string_view hostName)
{
	std::ostringstream ss;
	ss << "GET " << pageUrl << " HTTP/1.1\r\n"
		<< "Host: " << hostName << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n";
	auto request = ss.str();
	if (SOCKET_ERROR == send(connection, request.c_str(), request.length(), 0))
		throw WinsockSocketException();
}


// ----------------------------------------------------------------------------
void ReceiveResponse(SOCKET connection, std::ostream& out)
{
	const size_t bufsize = 512;
	char buffer[bufsize];

	// �������� ��������� � �� ����������� ��� ��������� �����.
	int received = recv(connection, buffer, bufsize - 1, 0);	// -1 - ����� ���� ���� ��������� '\0'
	if (!received)
		throw WinsockException("Can\'t read the page.");
	else if (received < 0)
		throw WinsockSocketException();

	// ��������� ��� �� ������ ������ �� ������� ���� 2��.
	// 30 �������� - ������ ���������� ��� ������� ������ ������.
	std::string firstLine(buffer, buffer + min(30, received));

	std::regex xSuccess(R"(HTTP\/[0-9]+\.[0-9]+\s+2[0-9]{2}(.|\r|\n)*)");
	if (!std::regex_match(firstLine, xSuccess))				// �� �������. ��� ������������ ��� ���������������, 404 � ������.
		throw WinsockException("The ansver is not 2xx.");

	// ��������� ������ ��������� ������ � ����������.
	buffer[received] = '\0';
	out << buffer;

	// TODO: ������ ��������� �� ������

	// ��������� �� ���������.
	do
	{
		received = recv(connection, buffer, bufsize - 1, 0);
		if (received >= 0)
		{
			buffer[received] = '\0';
			out << buffer;
		}
		else
			throw WinsockSocketException();

	} while (received);
}

// ----------------------------------------------------------------------------