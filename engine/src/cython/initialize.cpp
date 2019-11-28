#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


#include <memory>
#include <algorithm>
#include <thread>
#include <cuda_runtime.h>


#include <blazingdb/transport/io/reader_writer.h>


#include <blazingdb/io/Util/StringUtil.h>

#include <blazingdb/io/Config/BlazingContext.h>
#include <blazingdb/io/Library/Logging/Logger.h>
#include <blazingdb/io/Library/Logging/FileOutput.h>
// #include <blazingdb/io/Library/Logging/TcpOutput.h>
#include "blazingdb/io/Library/Logging/ServiceLogging.h"
// #include "blazingdb/io/Library/Network/NormalSyncSocket.h"

#include "Traits/RuntimeTraits.h"


#include "config/BlazingConfig.h"
#include "config/GPUManager.cuh"

#include "communication/CommunicationData.h"
#include "communication/network/Server.h"
#include "communication/network/Client.h"
#include <blazingdb/manager/Context.h>



std::string get_ip(const std::string &iface_name = "eth0") {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, iface_name.c_str(), IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    //printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    const std::string the_ip(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    return the_ip;
}

void initialize(int ralId, int gpuId, std::string network_iface_name,std::string ralHost, int ralCommunicationPort, bool singleNode){






      // std::cout << "Using the network interface: " + network_iface_name << std::endl;
      ralHost = get_ip(network_iface_name);

    //  std::cout << "RAL ID: " << ralId << std::endl;
    //  std::cout << "GPU ID: " << gpuId << std::endl;

    //  std::cout << "RAL HTTP communication host: " << ralHost << std::endl;
    //  std::cout << "RAL HTTP communication port: " << ralCommunicationPort << std::endl;

      // std::string loggingHost = "";
      // std::string loggingPort = 0;
      std::string loggingName = "";
      // if (argc == 11) {
      //   loggingHost = std::string(argv[9]);
      //   loggingPort = std::string(argv[10]);
      //   std::cout << "Logging host: " << ralHost << std::endl;
      //   std::cout << "Logging port: " << ralCommunicationPort << std::endl;
      // } else {
         loggingName = "RAL." + std::to_string(ralId) + ".log";
        std::cout << "Logging to "<<loggingName << std::endl;
       std::cout << "is singleNode? "<< singleNode << std::endl;

  // }
        ral::config::GPUManager::getInstance().initialize(gpuId);
        size_t total_gpu_mem_size = ral::config::GPUManager::getInstance().gpuMemorySize();
        assert(total_gpu_mem_size > 0);
        auto nthread = 4;
        blazingdb::transport::io::setPinnedBufferProvider(0.1 * total_gpu_mem_size, nthread);

        auto& communicationData = ral::communication::CommunicationData::getInstance();
        communicationData.initialize(
            ralId,
            "1.1.1.1",
            0,
            ralHost,
            ralCommunicationPort,
            0);

        ral::communication::network::Server::start(ralCommunicationPort);

      if (singleNode == true) {
          ral::communication::network::Server::getInstance().close();
      }
      auto& config = ral::config::BlazingConfig::getInstance();

      // NOTE IMPORTANT PERCY aqui es que pyblazing se entera que este es el ip del RAL en el _send de pyblazing
      config.setLogName(loggingName)
            .setSocketPath(ralHost);

      // std::cout << "Socket Name: " << config.getSocketPath() << std::endl;

      // if (loggingName != ""){
        auto output = new Library::Logging::FileOutput(config.getLogName(), false);
        Library::Logging::ServiceLogging::getInstance().setLogOutput(output);
        Library::Logging::ServiceLogging::getInstance().setNodeIdentifier(ralId);
      // } else {
      //   auto output = new Library::Logging::TcpOutput();
      //   std::shared_ptr<Library::Network::NormalSyncSocket> loggingSocket = std::make_shared<Library::Network::NormalSyncSocket>();
      //   loggingSocket->connect(loggingHost.c_str(), loggingPort.c_str());
      //   output.setSocket(loggingSocket);
      //   Library::Logging::ServiceLogging::getInstance().setLogOutput(output);
      // }

      // Init AWS S3 ... TODO see if we need to call shutdown and avoid leaks from s3 percy
      BlazingContext::getInstance()->initExternalSystems();

}

void finalize() {
    ral::communication::network::Client::closeConnections();
    ral::communication::network::Server::getInstance().close();
    cudaDeviceReset();
    exit(0);
}