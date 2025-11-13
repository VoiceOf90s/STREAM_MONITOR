#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include <iostream>
#include "MainWindow.h"


int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  
  // Настройка приложения
  app.setApplicationName("Twitch Stream Monitor");
  app.setApplicationVersion("2.4.0");
  app.setOrganizationName("GrimoireDev");
  
  // Установка темной темы
  app.setStyle(QStyleFactory::create("Fusion"));
  
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(43, 43, 43));
  darkPalette.setColor(QPalette::AlternateBase, QColor(37, 37, 37));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(58, 58, 58));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(187, 134, 252));
  darkPalette.setColor(QPalette::Highlight, QColor(187, 134, 252));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  
  app.setPalette(darkPalette);
  
  // Создание папок
  #ifdef _WIN32
    system("if not exist logs mkdir logs >nul 2>&1");
    system("if not exist stats mkdir stats >nul 2>&1");
  #else
    system("mkdir -p logs 2>/dev/null");
    system("mkdir -p stats 2>/dev/null");
  #endif
  
  std::cout << "===========================================================" << std::endl;
  std::cout << "  Twitch Stream Monitor v2.4 (GUI Edition)" << std::endl;
  std::cout << "  Priority-based monitoring | Max 2 tabs | System tray" << std::endl;
  std::cout << "===========================================================" << std::endl;
  std::cout << std::endl;
  
  try {
    MainWindow window;
    window.show();
    
    std::cout << "[GUI] Main window created successfully" << std::endl;
    std::cout << "[GUI] Application ready" << std::endl;
    std::cout << std::endl;
    
    return app.exec();
    
  } catch (const std::exception& e) {
    std::cerr << "[CRITICAL ERROR] Exception in GUI startup: " << e.what() << std::endl;
    return 1;
  }
}