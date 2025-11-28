import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), 'py-src'))
from ui import AudioControlApp

if __name__ == '__main__':
    app = AudioControlApp()
    app.run(None)
