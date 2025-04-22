import pandas as pd
import folium
from PySide6.QtWidgets import QWidget, QApplication, QVBoxLayout, QPushButton, QFileDialog, QLabel
from PySide6.QtWebEngineWidgets import QWebEngineView
import io
import sys


class FileSelector(QWidget):
    def __init__(self, main_app):
        super().__init__()
        self.main_app = main_app  # Reference to the main application
        self.file_path = None

        self.label = QLabel("No file selected")
        self.button = QPushButton("Open File")
        self.button.clicked.connect(self.open_file_dialog)

        layout = QVBoxLayout()
        layout.addWidget(self.label)
        layout.addWidget(self.button)
        self.setLayout(layout)

    def open_file_dialog(self):
        file_dialog = QFileDialog()
        file_path, _ = file_dialog.getOpenFileName(self, "Select File", "", "Text Files (*.txt)")

        if file_path:
            self.file_path = file_path
            self.hide()  # Hide the file selector instead of closing it
            self.main_app.on_file_selected(file_path)  # Notify the main app about the selected file


class MyApp(QWidget):
    def __init__(self, file_path, file_selector):
        super().__init__()
        self.file_selector = file_selector  # Keep a reference to the file selector

        self.df = pd.read_csv(file_path, header=None)
        self.lat_lng_df = self.df.iloc[:, 2:4]
        self.lat_lng_df.columns = ['lat', 'lng']
        self.setWindowTitle("Map")
        self.window_width, self.window_height = 1200, 800
        self.setMinimumSize(self.window_width, self.window_height)

        layout = QVBoxLayout()
        self.setLayout(layout)

        map_centor = [self.lat_lng_df['lat'].loc[0], self.lat_lng_df['lng'].loc[0]]  # center of the map by using the first point
        m = folium.Map(location=map_centor, zoom_start=15)

        folium.Marker([self.lat_lng_df['lat'].loc[0], self.lat_lng_df['lng'].loc[0]],  # start point
                      popup='Start', icon=folium.Icon(color='green')).add_to(m)

        folium.Marker([self.lat_lng_df['lat'].loc[len(self.lat_lng_df) - 1], self.lat_lng_df['lng'].loc[len(self.lat_lng_df) - 1]],  # end point
                      popup='End', icon=folium.Icon(color='red')).add_to(m)

        path_coords = [[row['lat'], row['lng']] for _, row in self.lat_lng_df.iterrows()]  # path coordinates

        folium.PolyLine(
            path_coords,
            color='blue',
            weight=5,
            opacity=0.7
        ).add_to(m)

        data = io.BytesIO()
        m.save(data, close_file=False)

        webView = QWebEngineView()
        webView.setHtml(data.getvalue().decode())
        layout.addWidget(webView)

    def closeEvent(self, event):
        # Show the file selector again when the map window is closed
        self.file_selector.show()
        event.accept()


class MainApp(QApplication):
    def __init__(self, argv):
        super().__init__(argv)
        self.file_selector = FileSelector(self)  # Pass reference to MainApp
        self.file_selector.show()

    def on_file_selected(self, file_path):
        self.my_app = MyApp(file_path, self.file_selector)
        self.my_app.show()


if __name__ == "__main__":
    app = MainApp(sys.argv)
    app.setStyleSheet('''
        QWidget {
                      font-size: 35px;
        }
    ''')

    try:
        sys.exit(app.exec())
    except SystemExit:
        print("Closing Window...")