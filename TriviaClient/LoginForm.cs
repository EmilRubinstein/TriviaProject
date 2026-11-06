using System;
using System.Drawing;
using System.Windows.Forms;

namespace TriviaClient
{
    public class LoginForm : Form
    {
        public Communicator Comm { get; private set; }
        public string LoggedInUsername { get; private set; }

        private TextBox txtHost, txtPort, txtUsername, txtPassword, txtEmail;
        private Label lblEmail, lblStatus;
        private Button btnLogin, btnSignup;
        private GroupBox grpConn, grpAuth;
        private bool _isLoginMode = true;

        public LoginForm()
        {
            Comm = new Communicator();
            Text = "Trivia Client - Login";
            Size = new Size(380, 420);
            StartPosition = FormStartPosition.CenterScreen;
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = false;
            BackColor = Color.FromArgb(45, 45, 48);
            ForeColor = Color.WhiteSmoke;
            Font = new Font("Segoe UI", 9.5f);

            // ── connection group ──
            grpConn = MakeGroup("Server Connection", 10, 10, 344, 75);
            Controls.Add(grpConn);

            grpConn.Controls.Add(MakeLabel("Host:", 10, 22, 40));
            txtHost = MakeTextBox("127.0.0.1", 55, 20, 160);
            grpConn.Controls.Add(txtHost);

            grpConn.Controls.Add(MakeLabel("Port:", 228, 22, 36));
            txtPort = MakeTextBox("8080", 268, 20, 66);
            grpConn.Controls.Add(txtPort);

            // ── auth group ──
            grpAuth = MakeGroup("Login", 10, 92, 344, 230);
            Controls.Add(grpAuth);

            grpAuth.Controls.Add(MakeLabel("Username:", 10, 24, 75));
            txtUsername = MakeTextBox("", 90, 22, 240);
            grpAuth.Controls.Add(txtUsername);

            grpAuth.Controls.Add(MakeLabel("Password:", 10, 58, 75));
            txtPassword = MakeTextBox("", 90, 56, 240);
            txtPassword.UseSystemPasswordChar = true;
            grpAuth.Controls.Add(txtPassword);

            lblEmail = MakeLabel("Email:", 10, 92, 75);
            grpAuth.Controls.Add(lblEmail);

            txtEmail = MakeTextBox("", 90, 90, 240);
            grpAuth.Controls.Add(txtEmail);

            btnLogin = MakeButton("Login", 10, 130, 155, Color.FromArgb(0, 122, 204));
            btnLogin.Click += (s, e) => DoAction(login: true);
            grpAuth.Controls.Add(btnLogin);

            btnSignup = MakeButton("Sign Up", 175, 130, 155, Color.FromArgb(70, 130, 70));
            btnSignup.Click += (s, e) => DoAction(login: false);
            grpAuth.Controls.Add(btnSignup);

            lblStatus = new Label
            {
                Location = new Point(10, 172),
                Size = new Size(320, 40),
                ForeColor = Color.Salmon,
                BackColor = Color.Transparent,
                Font = new Font("Segoe UI", 9f),
                Text = ""
            };
            grpAuth.Controls.Add(lblStatus);
        }

        private void DoAction(bool login)
        {
            lblStatus.Text = "";
            lblStatus.ForeColor = Color.Salmon;
            string host = txtHost.Text.Trim();
            string username = txtUsername.Text.Trim();
            string password = txtPassword.Text;

            if (!int.TryParse(txtPort.Text.Trim(), out int port) || port < 1024 || port > 65535)
            { lblStatus.Text = "Port must be between 1024 and 65535."; return; }
            if (string.IsNullOrEmpty(username) || string.IsNullOrEmpty(password))
            { lblStatus.Text = "Username and password are required."; return; }

            try
            {
                if (!Comm.IsConnected)
                    Comm.Connect(host, port);

                if (login)
                {
                    var (code, doc) = Comm.SendAndReceive(Codes.LOGIN, new { username, password });
                    if (doc.RootElement.GetProperty("status").GetInt32() == 1)
                    { LoggedInUsername = username; DialogResult = DialogResult.OK; Close(); }
                    else
                        lblStatus.Text = "Login failed. Check your credentials.";
                }
                else
                {
                    string email = txtEmail.Text.Trim();
                    if (string.IsNullOrEmpty(email)) { lblStatus.Text = "Email is required."; return; }

                    var (code, doc) = Comm.SendAndReceive(Codes.SIGN_UP, new { username, password, email });
                    if (doc.RootElement.GetProperty("status").GetInt32() == 1)
                    { LoggedInUsername = username; DialogResult = DialogResult.OK; Close(); }
                    else
                        lblStatus.Text = "Signup failed. Username may already be taken.";
                }
            }
            catch (Exception ex)
            {
                lblStatus.Text = "Connection error: " + ex.Message;
                Comm = new Communicator();
            }
        }

        // ── helpers ──
        private GroupBox MakeGroup(string text, int x, int y, int w, int h)
        {
            return new GroupBox
            {
                Text = text,
                Location = new Point(x, y),
                Size = new Size(w, h),
                ForeColor = Color.LightGray,
                BackColor = Color.FromArgb(55, 55, 58),
                Font = new Font("Segoe UI", 9f, FontStyle.Bold)
            };
        }

        private Label MakeLabel(string text, int x, int y, int w)
        {
            return new Label
            {
                Text = text,
                Location = new Point(x, y),
                Width = w,
                ForeColor = Color.WhiteSmoke,
                BackColor = Color.Transparent,
                TextAlign = ContentAlignment.MiddleRight,
                AutoSize = false
            };
        }

        private TextBox MakeTextBox(string text, int x, int y, int w)
        {
            return new TextBox
            {
                Text = text,
                Location = new Point(x, y),
                Width = w,
                BackColor = Color.FromArgb(30, 30, 30),
                ForeColor = Color.WhiteSmoke,
                BorderStyle = BorderStyle.FixedSingle
            };
        }

        private Button MakeButton(string text, int x, int y, int w, Color back)
        {
            var b = new Button
            {
                Text = text,
                Location = new Point(x, y),
                Size = new Size(w, 32),
                BackColor = back,
                ForeColor = Color.White,
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand
            };
            b.FlatAppearance.BorderSize = 0;
            return b;
        }
    }
}