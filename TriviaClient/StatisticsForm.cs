using System;
using System.Drawing;
using System.Text.Json;
using System.Windows.Forms;

namespace TriviaClient
{
    public class StatisticsForm : Form
    {
        private readonly Communicator _comm;
        private readonly string _username;
        private readonly bool _highScoreMode;

        public StatisticsForm(Communicator comm, string username, bool highScoreMode)
        {
            _comm = comm;
            _username = username;
            _highScoreMode = highScoreMode;
            InitializeComponents();
            FetchAndDisplay();
        }

        private void InitializeComponents()
        {
            Text = _highScoreMode ? "Trivia Client - High Scores" : "Trivia Client - My Statistics";
            Size = new Size(450, 500);
            StartPosition = FormStartPosition.CenterParent;
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            BackColor = Color.FromArgb(45, 45, 48);
            ForeColor = Color.WhiteSmoke;
            Font = new Font("Courier New", 9.5f);
        }

        private void FetchAndDisplay()
        {
            try
            {
                byte reqCode = _highScoreMode ? Codes.GET_HIGH_SCORE_REQUEST : Codes.GET_STATISTICS_REQUEST;
                var (code, doc) = _comm.SendAndReceive(reqCode);

                string key = _highScoreMode ? "HighScores" : "UserStatistics";

                if (!doc.RootElement.TryGetProperty(key, out JsonElement arr))
                {
                    ShowError("Invalid data received from server.");
                    return;
                }

                var mainPanel = new Panel
                {
                    Location = new Point(10, 10),
                    Size = new Size(415, 410),
                    AutoScroll = true,
                    BackColor = Color.Transparent
                };
                Controls.Add(mainPanel);

                var titleLabel = new Label
                {
                    Text = _highScoreMode ? "HIGH SCORES" : $"STATISTICS FOR: {_username.ToUpper()}",
                    Location = new Point(0, 0),
                    Size = new Size(400, 35),
                    Font = new Font("Courier New", 12f, FontStyle.Bold),
                    ForeColor = Color.Gold,
                    TextAlign = ContentAlignment.MiddleCenter
                };
                mainPanel.Controls.Add(titleLabel);

                var listBox = new ListBox
                {
                    Location = new Point(0, 40),
                    Size = new Size(400, 350),
                    BackColor = Color.FromArgb(30, 30, 30),
                    ForeColor = Color.WhiteSmoke,
                    BorderStyle = BorderStyle.FixedSingle,
                    SelectionMode = SelectionMode.None,
                    Font = new Font("Courier New", 10f),
                    IntegralHeight = false
                };
                mainPanel.Controls.Add(listBox);

                string[] medals = { "1st", "2nd", "3rd" };
                int index = 0;
                bool hasData = false;

                foreach (var item in arr.EnumerateArray())
                {
                    hasData = true;
                    string itemText = item.GetString();

                    if (_highScoreMode && index < 3)
                    {
                        listBox.Items.Add($"{medals[index],-8} {itemText}");
                    }
                    else if (_highScoreMode)
                    {
                        listBox.Items.Add($"   #{index + 1}   {itemText}");
                    }
                    else
                    {
                        listBox.Items.Add(itemText);
                    }
                    index++;
                }

                if (!hasData)
                {
                    listBox.Items.Add("No data available yet.");
                    listBox.Items.Add("");
                    listBox.Items.Add(_highScoreMode ?
                        "Play some games to see high scores!" :
                        "Play some games to see your statistics!");
                }

                var btnClose = new Button
                {
                    Text = "Close",
                    Location = new Point(0, 395),
                    Size = new Size(400, 35),
                    BackColor = Color.FromArgb(80, 80, 84),
                    ForeColor = Color.White,
                    FlatStyle = FlatStyle.Flat,
                    Cursor = Cursors.Hand,
                    Font = new Font("Segoe UI", 10f, FontStyle.Bold)
                };
                btnClose.FlatAppearance.BorderSize = 0;
                btnClose.Click += (s, e) => Close();
                mainPanel.Controls.Add(btnClose);
            }
            catch (Exception ex)
            {
                ShowError("Could not load data: " + ex.Message);
            }
        }

        private void ShowError(string message)
        {
            var lblError = new Label
            {
                Text = message,
                Location = new Point(20, 20),
                Size = new Size(380, 80),
                ForeColor = Color.Salmon,
                TextAlign = ContentAlignment.MiddleCenter,
                Font = new Font("Segoe UI", 10f)
            };
            Controls.Add(lblError);

            var btnClose = new Button
            {
                Text = "Close",
                Location = new Point(150, 110),
                Size = new Size(120, 35),
                BackColor = Color.FromArgb(80, 80, 84),
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand
            };
            btnClose.FlatAppearance.BorderSize = 0;
            btnClose.Click += (s, e) => Close();
            Controls.Add(btnClose);
        }
    }
}