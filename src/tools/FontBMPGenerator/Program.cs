using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Threading.Tasks;

namespace FontGenerator
{
    class Program
    {
        static void Main(string[] args)
        {
            Bitmap bmp = new Bitmap(2400, 100, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            RectangleF rectf = new RectangleF(0, 0, bmp.Width, bmp.Height);

            Graphics g = Graphics.FromImage(bmp);

            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighQuality;
            g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBilinear;
            g.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.HighQuality;
            string fontText = "0123456789ABCDEFGHIGKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            Font stringFont = new Font("Tahoma", 28);
            //g.DrawString(fontText, stringFont, Brushes.Black, rectf);
            SizeF totalSize = g.MeasureString(fontText, stringFont);
            g.DrawString(fontText, stringFont, Brushes.Black, 0, totalSize.Height);

            LinkedList<SizeF> rectList = new LinkedList<SizeF>();
            float totalLength = 0.0f;
            for (int i = 0; i < fontText.Length; ++i)
            {
                char ch = fontText[i];
                SizeF size = new SizeF();
                size = g.MeasureString(ch.ToString(), stringFont, totalSize);
                rectList.AddLast(size);
                g.DrawRectangle(new Pen(Color.Red, 1), totalLength, 0, size.Width, size.Height);
                g.DrawString(ch.ToString(), stringFont, Brushes.Black, totalLength, 0.0f);
                totalLength += size.Width;
            }

            g.Flush();

            bmp.Save(@"D:\temp\test.png", System.Drawing.Imaging.ImageFormat.Png);
        }
    }
}
