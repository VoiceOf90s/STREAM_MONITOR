#ifndef HUMAN_BEHAVIOR_H
#define HUMAN_BEHAVIOR_H

#include <random>
#include <chrono>
#include <thread>
#include <string>


// Класс для имитации человеческого поведения
class HumanBehavior {
private:
    std::mt19937 randomGenerator;
    
    // Генераторы случайных задержек
    int GenerateThinkingDelay() const;      // 500-2000ms
    int GenerateTypingDelay() const;        // 100-300ms за символ
    int GenerateMouseMoveDelay() const;     // 50-150ms
    int GeneratePageLoadWait() const;       // 1000-3000ms
    
    // Случайные User-Agents
    std::string GetRandomUserAgent() const;
    
    // Случайные Accept-Language заголовки
    std::string GetRandomAcceptLanguage() const;
    
    // Случайный порядок заголовков
    std::vector<std::string> GetRandomizedHeaders() const;


public:
    HumanBehavior();
    
    // Имитация "думания" перед действием
    void SimulateThinking() const;
    
    // Имитация набора текста
    void SimulateTyping(const std::string& text) const;
    
    // Имитация движения мыши
    void SimulateMouseMove() const;
    
    // Имитация ожидания загрузки страницы
    void SimulatePageLoad() const;
    
    // Случайная задержка в диапазоне
    void RandomDelay(int minMs, int maxMs) const;
    
    // Получить случайный User-Agent для текущей сессии
    std::string GetSessionUserAgent() const;
    
    // Получить случайные заголовки HTTP
    std::vector<std::pair<std::string, std::string>> GetHumanLikeHeaders() const;
    
    // Решить, стоит ли делать дополнительный запрос (для маскировки)
    bool ShouldMakeExtraRequest() const;  // 20% шанс
    
    // Генерация случайного referer
    std::string GetRandomReferer() const;
};

#endif // HUMAN_BEHAVIOR_H