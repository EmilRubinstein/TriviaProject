#pragma once
#include <string>
#include <vector>

class Question
{
public:
    Question();
    Question(const std::string& question, const std::vector<std::string>& possibleAnswers, int correctAnswerId);

    std::string getQuestion() const;
    std::vector<std::string> getPossibleAnswers() const;
    int getCorrectAnswerId() const;

private:
    std::string m_question;
    std::vector<std::string> m_possibleAnswers;
    int m_correctAnswerId;
};