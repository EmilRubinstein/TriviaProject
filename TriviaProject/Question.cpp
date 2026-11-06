#include "Question.h"

Question::Question()
    : m_question(""), m_possibleAnswers({}), m_correctAnswerId(0)
{}

Question::Question(const std::string& question, const std::vector<std::string>& possibleAnswers, int correctAnswerId)
    : m_question(question), m_possibleAnswers(possibleAnswers), m_correctAnswerId(correctAnswerId)
{}

std::string Question::getQuestion() const
{
    return m_question;
}

std::vector<std::string> Question::getPossibleAnswers() const
{
    return m_possibleAnswers;
}

int Question::getCorrectAnswerId() const
{
    return m_correctAnswerId;
}