using System;
using System.Drawing;
using System.Text.Json;
using System.Windows.Forms;
using System.Threading.Tasks;

namespace TriviaClient
{
    public class GameForm : Form
    {
        private readonly Communicator _comm;
        private readonly string _username;
        private readonly int _roomId;

        private Label lblQuestion;
        private Button[] answerButtons;
        private Label lblProgress;
        private Label lblTimer;
        private Label lblStatus;
        private System.Windows.Forms.Timer _timer;
        private int _timeRemaining;
        private int _timePerQuestion;
        private int _questionsAnswered = 0;
        private int _correctCount = 0;
        private int _totalQuestions;
        private bool _gameActive = true;
        private bool _waitingForResults = false;
        private bool _isClosing = false;
        private bool _leaveSent = false;
        private char[] positions = { 'A', 'B', 'C', 'D' };
        private bool _returnedToMenu = false;

        public GameForm(Communicator comm, string username, int roomId, int timePerQuestion, int totalQuestions)
        {
            _comm = comm;
            _username = username;
            _roomId = roomId;
            _timePerQuestion = timePerQuestion;
            _totalQuestions = totalQuestions;

            InitializeComponents();
            this.FormClosing += GameForm_FormClosing;
            this.Shown += async (s, e) => await StartGame();
        }

        private void InitializeComponents()
        {
            this.Text = "Trivia Game";
            this.Size = new Size(650, 580);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.BackColor = Color.FromArgb(45, 45, 48);

            // Question label
            lblQuestion = new Label()
            {
                Location = new Point(20, 20),
                Size = new Size(590, 80),
                Font = new Font("Segoe UI", 14, FontStyle.Bold),
                ForeColor = Color.White,
                TextAlign = ContentAlignment.MiddleCenter,
                BackColor = Color.FromArgb(30, 30, 35),
                Text = "Waiting for first question..."
            };
            this.Controls.Add(lblQuestion);

            // Answer buttons
            answerButtons = new Button[4];
            int[] yPositions = { 120, 185, 250, 315 };

            for (int i = 0; i < 4; i++)
            {
                answerButtons[i] = new Button()
                {
                    Text = $"{positions[i]}. ",
                    Location = new Point(50, yPositions[i]),
                    Size = new Size(550, 50),
                    Font = new Font("Segoe UI", 11),
                    BackColor = Color.FromArgb(60, 60, 65),
                    ForeColor = Color.White,
                    FlatStyle = FlatStyle.Flat,
                    Tag = i + 1,
                    Enabled = false
                };
                answerButtons[i].FlatAppearance.BorderSize = 0;
                answerButtons[i].Click += AnswerButton_Click;
                this.Controls.Add(answerButtons[i]);
            }

            // Progress label
            lblProgress = new Label()
            {
                Location = new Point(20, 390),
                Size = new Size(350, 25),
                Font = new Font("Segoe UI", 10),
                ForeColor = Color.LightGray,
                Text = $"Question: 0 / {_totalQuestions}"
            };
            this.Controls.Add(lblProgress); 

            // Timer label
            lblTimer = new Label()
            {
                Location = new Point(480, 390),
                Size = new Size(150, 45),
                Font = new Font("Segoe UI", 18, FontStyle.Bold),
                ForeColor = Color.Gold,
                TextAlign = ContentAlignment.MiddleRight,
                Text = ""
            };
            this.Controls.Add(lblTimer);

            lblStatus = new Label()
            {
                Location = new Point(20, 460),
                Size = new Size(590, 60),
                Font = new Font("Segoe UI", 9),
                ForeColor = Color.Salmon,
                TextAlign = ContentAlignment.MiddleCenter,
                Text = ""
            };
            this.Controls.Add(lblStatus);

            _timer = new System.Windows.Forms.Timer();
            _timer.Interval = 1000;
            _timer.Tick += Timer_Tick;
        }

        private async Task StartGame()
        {
            await GetNextQuestion();
        }

        private async Task GetNextQuestion()
        {
            if (!_gameActive) return;

            try
            {
                lblStatus.Text = "Loading next question...";
                foreach (var btn in answerButtons)
                    btn.Enabled = false;

                var (code, response) = _comm.SendAndReceive(Codes.GET_QUESTION_REQUEST, new { });

                if (!response.RootElement.TryGetProperty("status", out JsonElement st) || st.GetInt32() != 1)
                {
                    await GetResults();
                    return;
                }

                string question = response.RootElement.GetProperty("question").GetString();
                lblQuestion.Text = question;

                var answerList = new List<(int originalId, string text)>();
                foreach (var answerProp in response.RootElement.GetProperty("answers").EnumerateObject())
                {
                    if (int.TryParse(answerProp.Name, out int ansNum) && ansNum >= 1 && ansNum <= 4)
                        answerList.Add((ansNum - 1, answerProp.Value.GetString()));
                }

                var rng = new Random();
                answerList = answerList.OrderBy(_ => rng.Next()).ToList();

                for (int i = 0; i < answerList.Count; i++)
                {
                    answerButtons[i].Text = $"{positions[i]}. {answerList[i].text}";
                    answerButtons[i].Tag = answerList[i].originalId;
                    answerButtons[i].Enabled = true;
                }

                lblProgress.Text = $"Question: {_questionsAnswered + 1} / {_totalQuestions}";
                _timeRemaining = _timePerQuestion;
                UpdateTimerDisplay();
                _timer.Start();
                lblStatus.Text = "";
            }
            catch (Exception ex)
            {
                lblStatus.Text = $"Error loading question: {ex.Message}";
            }
        }

        private async void AnswerButton_Click(object sender, EventArgs e)
        {
            Button clickedBtn = sender as Button;
            if (clickedBtn == null || !clickedBtn.Enabled) return;

            foreach (var btn in answerButtons)
                btn.Enabled = false;
            _timer.Stop();

            int answerId = (int)clickedBtn.Tag;
            float answerTime = _timePerQuestion - _timeRemaining;
            if (answerTime < 0) answerTime = 0;

            try
            {
                lblStatus.Text = "Submitting answer...";

                var (code, doc) = _comm.SendAndReceive(Codes.SUBMIT_ANSWER_REQUEST, new
                {
                    answerId = answerId,
                    answerTime = answerTime
                });

                _questionsAnswered++;
                if (_questionsAnswered < _totalQuestions)
                    lblProgress.Text = $"Question: {_questionsAnswered + 1} / {_totalQuestions}";

                await Task.Delay(500);
                await GetNextQuestion();
            }
            catch (Exception ex)
            {
                lblStatus.Text = $"Error submitting answer: {ex.Message}";
                lblStatus.ForeColor = Color.Salmon;
                foreach (var btn in answerButtons)
                    btn.Enabled = true;
                _timer.Start();
            }
        }

        private async Task GetResults()
        {
            if (_waitingForResults) return;
            _waitingForResults = true;
            _gameActive = false;
            _timer.Stop();

            lblStatus.Text = "Game finished! Waiting for other players...";
            lblStatus.ForeColor = Color.LightGray;

            int attempts = 0;
            while (attempts < 30) // max 30 seconds
            {
                try
                {
                    await Task.Delay(1000);
                    var (code, doc) = _comm.SendAndReceive(Codes.GET_GAME_RESULT_REQUEST, new { });

                    if (doc.RootElement.TryGetProperty("status", out JsonElement status) && status.GetInt32() == 1)
                    {
                        if (doc.RootElement.TryGetProperty("results", out JsonElement results))
                        {
                            ShowResults(results);
                            return;
                        }
                    }
                }
                catch (Exception ex)
                {
                    ShowResultsError(ex.Message);
                    return;
                }
                attempts++;
            }

            ShowResultsError("Timed out waiting for results.");
        }

        private void ShowResults(JsonElement results)
        {
            this.Controls.Clear();
            this.BackColor = Color.FromArgb(45, 45, 48);

            Label title = new Label()
            {
                Text = " GAME RESULTS ",
                Location = new Point(20, 20),
                Size = new Size(590, 45),
                Font = new Font("Segoe UI", 18, FontStyle.Bold),
                ForeColor = Color.Gold,
                TextAlign = ContentAlignment.MiddleCenter
            };
            this.Controls.Add(title);

            ListBox listResults = new ListBox()
            {
                Location = new Point(50, 80),
                Size = new Size(530, 340),
                Font = new Font("Courier New", 11),
                BackColor = Color.FromArgb(30, 30, 35),
                ForeColor = Color.White,
                BorderStyle = BorderStyle.None
            };
            this.Controls.Add(listResults);

            listResults.Items.Add("┌───────────────┬────────────┬──────────┬─────────────────┐");
            listResults.Items.Add("│ Player        │ Correct    │ Wrong    │ Avg Time (sec)  │");
            listResults.Items.Add("├───────────────┼────────────┼──────────┼─────────────────┤");

            foreach (var playerResult in results.EnumerateArray())
            {
                string player = "";
                int correct = 0;
                int wrong = 0;
                float avgTime = 0;

                if (playerResult.TryGetProperty("username", out JsonElement userElem))
                    player = userElem.GetString();
                if (playerResult.TryGetProperty("correctAnswersCount", out JsonElement correctElem))
                    correct = correctElem.GetInt32();
                if (playerResult.TryGetProperty("wrongAnswerCount", out JsonElement wrongElem))
                    wrong = wrongElem.GetInt32();
                if (playerResult.TryGetProperty("averageAnswerTime", out JsonElement avgElem))
                    avgTime = (float)avgElem.GetDouble();

                listResults.Items.Add($"│ {player,-13} │ {correct,10} │ {wrong,8} │ {avgTime,15:F2} │");
            }

            listResults.Items.Add("└───────────────┴────────────┴──────────┴─────────────────┘");

            Button btnClose = new Button()
            {
                Text = "Return to Main Menu",
                Location = new Point(200, 440),
                Size = new Size(250, 45),
                BackColor = Color.FromArgb(0, 122, 204),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Font = new Font("Segoe UI", 11, FontStyle.Bold),
                Cursor = Cursors.Hand
            };
            btnClose.Click += (s, e) => { _returnedToMenu = true; this.DialogResult = DialogResult.OK; };
            this.Controls.Add(btnClose);
        }

        private void ShowResultsError(string error)
        {
            this.Controls.Clear();

            Label errorLabel = new Label()
            {
                Text = $"Error getting results: {error}",
                Location = new Point(20, 20),
                Size = new Size(590, 40),
                Font = new Font("Segoe UI", 12),
                ForeColor = Color.Salmon,
                TextAlign = ContentAlignment.MiddleCenter
            };
            this.Controls.Add(errorLabel);

            Button btnClose = new Button()
            {
                Text = "Return to Main Menu",
                Location = new Point(200, 440),
                Size = new Size(250, 45),
                BackColor = Color.FromArgb(0, 122, 204),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Font = new Font("Segoe UI", 11, FontStyle.Bold),
                Cursor = Cursors.Hand
            };
            btnClose.Click += (s, e) => { _returnedToMenu = true; this.DialogResult = DialogResult.OK; };
            this.Controls.Add(btnClose);
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            _timeRemaining--;
            UpdateTimerDisplay();

            if (_timeRemaining <= 0)
            {
                _timer.Stop();
                foreach (var btn in answerButtons)
                    btn.Enabled = false;
                AutoSubmitTimeout();
            }
        }

        private async void AutoSubmitTimeout()
        {
            try
            {
                lblStatus.Text = "Time's up! Moving to next question...";

                var (code, doc) = _comm.SendAndReceive(Codes.SUBMIT_ANSWER_REQUEST, new
                {
                    answerId = -1,
                    answerTime = (float)_timePerQuestion
                });

                _questionsAnswered++;
                if (_questionsAnswered < _totalQuestions)
                    lblProgress.Text = $"Question: {_questionsAnswered + 1} / {_totalQuestions}";

                await Task.Delay(1000);
                await GetNextQuestion();
            }
            catch (Exception ex)
            {
                lblStatus.Text = $"Error: {ex.Message}";
            }
        }

        private void UpdateTimerDisplay()
        {
            if (_timeRemaining >= 0)
                lblTimer.Text = $"{_timeRemaining}s";
        }

        private void GameForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            _timer?.Stop();

            if (_leaveSent || _returnedToMenu) return;
            _leaveSent = true;

            try
            {
                if (_gameActive && !_waitingForResults)
                    _comm.Send(Codes.LEAVE_GAME_REQUEST, new { });
                else
                    _comm.SendAndReceive(Codes.SIGNOUT_REQUEST);
            }
            catch { }
        }
    }
}