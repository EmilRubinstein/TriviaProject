using System;
using System.Windows.Forms;

namespace TriviaClient
{
    internal static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            while (true)
            {
                var loginForm = new LoginForm();
                if (loginForm.ShowDialog() != DialogResult.OK)
                    break;

                var comm = loginForm.Comm;
                var username = loginForm.LoggedInUsername;

                var menuResult = new MainMenuForm(comm, username).ShowDialog();

                if (menuResult != DialogResult.Retry)
                {
                    comm.Disconnect();
                    break;
                }
                // Retry = signed out, loop back to login
            }
        }
    }
}