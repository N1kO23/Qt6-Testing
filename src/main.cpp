#include <QApplication>
#include <QDateTime>
#include <QPushButton>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QTimer>

#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// Returns the cpu usage as a percentage
double get_cpu_usage()
{
  std::ifstream stat_file("/proc/stat");
  std::string line;
  std::getline(stat_file, line); // read the first line which contains CPU usage info
  stat_file.close();

  // parse the line to get CPU usage values
  unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  sscanf(line.c_str(), "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
         &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
  unsigned long long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
  unsigned long long usage_time = total_time - idle;
  double cpu_usage = static_cast<double>(usage_time) / total_time * 100;

  return cpu_usage;
}

// Returns the current RAM usage in kB
unsigned long long get_ram_usage()
{
  std::ifstream meminfo_file("/proc/meminfo");
  std::string line;
  unsigned long long mem_total, mem_free, buffers, cached, slab, mem_available;

  // read the /proc/meminfo file line by line and extract the relevant memory information
  while (std::getline(meminfo_file, line))
  {
    if (sscanf(line.c_str(), "MemTotal: %llu kB", &mem_total) == 1)
    {
      continue;
    }
    else if (sscanf(line.c_str(), "MemFree: %llu kB", &mem_free) == 1)
    {
      continue;
    }
    else if (sscanf(line.c_str(), "Buffers: %llu kB", &buffers) == 1)
    {
      continue;
    }
    else if (sscanf(line.c_str(), "Cached: %llu kB", &cached) == 1)
    {
      continue;
    }
    else if (sscanf(line.c_str(), "Slab: %llu kB", &slab) == 1)
    {
      break;
    }
  }

  mem_available = mem_free + buffers + cached + slab;

  return mem_total - mem_available;
}

// Returns the ip address of the socket that was used to connect to the google dns server
std::string get_ip_address()
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1)
  {
    std::cerr << "Error opening socket" << std::endl;
    return "";
  }

  const char *googleDnsIp = "8.8.8.8";
  uint16_t dnsPort = 53;

  struct sockaddr_in serv
  {
  };
  serv.sin_family = AF_INET;
  serv.sin_addr.s_addr = inet_addr(googleDnsIp);
  serv.sin_port = htons(dnsPort);

  if (connect(fd, (const sockaddr *)&serv, sizeof(serv)) == -1)
  {
    std::cerr << "Error connecting to server" << std::endl;
    close(fd);
    return "";
  }

  struct sockaddr_in name
  {
  };
  socklen_t namelen = sizeof(name);
  if (getsockname(fd, (struct sockaddr *)&name, &namelen) == -1)
  {
    std::cerr << "Error getting socket name" << std::endl;
    close(fd);
    return "";
  }

  char buffer[INET_ADDRSTRLEN];
  const char *ip = inet_ntop(AF_INET, &name.sin_addr, buffer, INET_ADDRSTRLEN);
  if (ip == nullptr)
  {
    std::cerr << "Error converting IP address" << std::endl;
    close(fd);
    return "";
  }

  close(fd);
  return std::string(ip);
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv); // Create the application object

  // Get the current user name
  QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString userName = QDir(homeDir).dirName();

  // Create the hello user label
  QLabel label("Hello, " + userName + "!");
  label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  label.setMargin(10);

  // Create a label for the current time
  QLabel time("Current local time: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
  QTimer timer; // Create a timer

  // Create a label for the ip address
  QLabel ipAddr;

  // Layout for ip address and time
  QHBoxLayout timeAndIpAddressLayout;
  timeAndIpAddressLayout.addWidget(&time, 0, Qt::AlignCenter);
  timeAndIpAddressLayout.addWidget(&ipAddr, 0, Qt::AlignCenter);

  // Create a label for the cpu usage
  QLabel cpuUsage("CPU usage: " + QString::number(round(get_cpu_usage() * 100) / 100.0) + "%");

  // Create a label for the cpu usage
  QLabel ramUsage("RAM usage: " + QString::number(get_ram_usage() / 1024) + "MB");

  // Layout for cpu usage
  QHBoxLayout cpuRamUsageLayout;
  cpuRamUsageLayout.addWidget(&cpuUsage, 0, Qt::AlignCenter);
  cpuRamUsageLayout.addWidget(&ramUsage, 0, Qt::AlignCenter);

  // Load the image and create a rounded pixmap
  QPixmap pixmap(":/images/images/test.png");
  QSize size = pixmap.size();
  QPixmap roundedPixmap(size);
  roundedPixmap.fill(Qt::transparent);
  QPainter painter(&roundedPixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  QPainterPath path;
  path.addRoundedRect(QRectF(QPointF(0, 0), size), 10, 10);
  painter.setClipPath(path);
  painter.drawPixmap(0, 0, pixmap);
  painter.setClipping(false);
  painter.end();

  // Create a label for the image
  QLabel imageLabel;
  imageLabel.setPixmap(pixmap);
  imageLabel.setMask(roundedPixmap.mask());

  // Load another image into pixmap
  QPixmap yukiPixmap(":/images/images/yuki.jpg");
  QLabel yukiLabel;
  QSize labelSize = yukiLabel.size();
  // Scale the pixmap to cover the entire label area
  yukiLabel.setPixmap(yukiPixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

  // Create a button to open new window with another image
  QPushButton button("Click me!");
  button.setGeometry(100, 100, 100, 50);

  // Handler for the button click
  QObject::connect(&button, &QPushButton::clicked, [&]()
                   { qDebug() << "Button clicked!";
                   yukiLabel.show(); });

  // Create the main layout and add widgets into it
  QVBoxLayout layout;
  layout.addWidget(&label, 0, Qt::AlignTop | Qt::AlignHCenter);
  layout.addLayout(&timeAndIpAddressLayout);
  layout.addLayout(&cpuRamUsageLayout);
  layout.addWidget(&imageLabel, 0, Qt::AlignCenter);
  layout.addWidget(&button, 0, Qt::AlignBottom);

  // Create the window and set the layout
  QWidget window;
  window.setLayout(&layout);
  window.setWindowTitle("Hello, World!");
  window.show();

  ipAddr.setText("Your IP Address: " + QString::fromStdString(get_ip_address())); // Fetch IP address from google

  timer.setInterval(1000); // Update the time every second
  QObject::connect(&timer, &QTimer::timeout, [&]()
                   { time.setText("Current local time: " + QDateTime::currentDateTime().toString("hh:mm:ss")); 
                      cpuUsage.setText("CPU usage: " + QString::number(round(get_cpu_usage() * 100) / 100.0) + "%"); 
                      ramUsage.setText("RAM usage: " + QString::number(get_ram_usage() / 1024) + "MB"); });
  timer.start();

  return app.exec(); // Start the main event loop
}