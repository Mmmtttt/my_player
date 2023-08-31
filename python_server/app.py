from flask import Flask, render_template, request, send_from_directory, redirect, url_for
import os
import mimetypes
import sys
import subprocess

app = Flask(__name__)
#UPLOAD_FOLDER = os.getcwd()

UPLOAD_FOLDER = "C:/"
CURR_FOLDER = UPLOAD_FOLDER

cmd_args = sys.argv
if len(cmd_args) >= 2:
    UPLOAD_FOLDER = cmd_args[1]
else:
    UPLOAD_FOLDER = UPLOAD_FOLDER

@app.route('/')
def index():
    files = os.listdir(UPLOAD_FOLDER)
    folders = [f for f in files if os.path.isdir(os.path.join(UPLOAD_FOLDER, f))]
    files = [f for f in files if not os.path.isdir(os.path.join(UPLOAD_FOLDER, f))]
    return render_template('index.html', folders=folders, files=files)

@app.route('/view_folder/<path:folder>')
def view_folder(folder):
    if folder == "..":
        parent_folder = os.path.dirname(UPLOAD_FOLDER)
        CURR_FOLDER = parent_folder
        return redirect(url_for('view_folder', folder=parent_folder))

    folder_path = os.path.join(UPLOAD_FOLDER, folder)
    all_files = os.listdir(folder_path)
    sub_folders = [f for f in all_files if os.path.isdir(os.path.join(folder_path, f))]
    files = [f for f in all_files if not os.path.isdir(os.path.join(folder_path, f))]
    CURR_FOLDER = folder
    return render_template('folder_view.html', folder=folder, sub_folders=sub_folders, files=files, url_for=url_for)

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return "No file part"
    file = request.files['file']
    if file.filename == '':
        return "No selected file"
    file.save(os.path.join(UPLOAD_FOLDER, file.filename))
    return redirect('/')

@app.route('/download/<path:filename>')
def download_file(filename):
    return send_from_directory(UPLOAD_FOLDER, filename)

@app.route('/delete/<path:filename>')
def delete_file(filename):
    try:
        os.remove(os.path.join(UPLOAD_FOLDER, filename))
        return "File deleted successfully"
    except FileNotFoundError:
        return "File not found"

@app.route('/rename/<path:filename>', methods=['POST'])
def rename_file(filename):
    new_name = request.form['new_name']
    try:
        os.rename(os.path.join(UPLOAD_FOLDER, filename), os.path.join(UPLOAD_FOLDER, new_name))
        return "File renamed successfully"
    except FileNotFoundError:
        return "File not found"

@app.route('/copy/<path:filename>', methods=['POST'])
def copy_file(filename):
    new_name = request.form['new_name']
    try:
        shutil.copy2(os.path.join(UPLOAD_FOLDER, filename), os.path.join(UPLOAD_FOLDER, new_name))
        return "File copied successfully"
    except FileNotFoundError:
        return "File not found"

@app.route('/view/<path:filename>')
def view_file(filename):
    mime, _ = mimetypes.guess_type(filename)
    if mime and mime.startswith('text'):
        with open(os.path.join(UPLOAD_FOLDER, filename), 'r') as file:
            content = file.read()
            return render_template('text_viewer.html', filename=filename, content=content)
    elif mime and mime.startswith('image'):
        file_path = os.path.dirname(filename)
        dir = os.path.join(UPLOAD_FOLDER, file_path)
        all_files = os.listdir(dir)
        print(all_files)
        image_files = [f for f in all_files if any(f.lower().endswith(ext) for ext in ('.jpg', '.jpeg', '.png', '.gif', '.ico'))]

        current_index = image_files.index(os.path.basename(filename))
        prev_image = image_files[current_index - 1] if current_index > 0 else None
        next_image = image_files[current_index + 1] if current_index < len(image_files) - 1 else None

        if prev_image is not None:
            prev_image = file_path + '/' + prev_image
        if next_image is not None:
            next_image = file_path + '/' + next_image

        return render_template('image_viewer.html', filename=filename, prev_image=prev_image, next_image=next_image)
    elif mime and mime.startswith('video'):
        return render_template('video_player.html', filename=filename)
    else:
        return "Unsupported file type"

@app.route('/download_torrent', methods=['POST'])
def download_torrent():
    magnet_link = request.form['magnet_link']
    # 使用 webtorrent-cli 下载磁力链接内容
    try:
        subprocess.run(['..\\libs_win\\npm\\webtorrent.cmd', magnet_link], check=True)
        return "Download completed"
    except subprocess.CalledProcessError:
        return "Error during download"  

@app.route('/play_torrent', methods=['POST'])
def play_torrent():
    magnet_link = request.form['magnet_link']
    return render_template('torrent_video.html', magnet_link=magnet_link)

#if __name__ == '__main__':
    #app.run(debug=True)
app.run(host='0.0.0.0', port=8080)