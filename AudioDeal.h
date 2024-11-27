#ifndef AUDIODEAL_H
#define AUDIODEAL_H

#include <QAudioBuffer>
#include <QDebug>
#include "fftw3.h"

template <class T>
QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);
    for (int i = 0; i < frames; ++i) //遍历所有帧
    {
        for (int j = 0; j < channels; ++j) //遍历通道
        {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values[j])
                max_values[j] = value;
        }
    }
    return max_values;
}

template <class T>
void getBuffers(QVector<QVector<qreal>> &data, const T *buffer, int frames, int channels)
{
    for (int i = 0; i < frames; ++i) //遍历所有帧
    {
        for (int j = 0; j < channels; ++j) //遍历通道
        {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            data[j][i] = value;
        }
    }
}

qreal getPeakValue(const QAudioFormat &format)
{
    // 检查音频格式是否有效
    if (!format.isValid())
        return qreal(0);

    // 检查音频编码是否为PCM
    if (format.codec() != "audio/pcm")
        return qreal(0);

    // 根据样本类型计算峰值值
    switch (format.sampleType())
    {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        // 对于浮点样本，只支持32位，且返回一个略大于1的值
        if (format.sampleSize() != 32)
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        // 对于有符号整数样本，根据样本大小返回相应的最大值
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        // 对于无符号整数样本，根据样本大小返回相应的最大值
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    // 如果没有匹配到任何已知情况，返回0
    return qreal(0);
}

void Standardardize(QVector<qreal>& vec, qreal peakValue) {
    for(auto& x : vec)
        x /= peakValue;
}

QVector<qreal> getBufferLevels(const QAudioBuffer &buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;
    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    // 根据样本类型和大小，计算每个通道的电平值
    switch (buffer.format().sampleType())
    {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        // 标准化电平值
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values[i] - peak_value/2) / (peak_value/2);
        break;
    case QAudioFormat::Float:
        // 处理浮点型样本，支持32位
        if (buffer.format().sampleSize() == 32)
        {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            // 标准化电平值
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        // 处理有符号整型样本，支持32位、16位和8位
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        // 标准化电平值
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }
    return values;
}


//快速傅里叶变换
void FFT(qreal *y,int N,qreal *FFT) {

    fftw_complex *in = NULL;
    fftw_complex *out = NULL;
    fftw_plan p;
    //分配内存空间
    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    // 创建句柄
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    // 输入
    for (int i=0; i<N; i++)
    {
        in[i][0] = y[i];
        in[i][1] = 0;
    }
    //执行
    fftw_execute(p);
    fftw_destroy_plan(p);
    // 输出
    for (int i=0; i<N; i++)
    {
        FFT[i]= sqrt((out[i][0])*(out[i][0])+(out[i][1])*(out[i][1]));
    }
    fftw_free(in);
    fftw_free(out);
}

//计算各个声道的频谱
QVector<QVector<qreal>> calcSpectrum(const QAudioBuffer &buffer) {
    //计算该格式下的最大量化值
    qreal peakValue = getPeakValue(buffer.format());
    int channelCount = buffer.format().channelCount();
    int N = buffer.frameCount(); //离散点数量
    QVector<QVector<qreal>> res(channelCount,QVector<qreal>(N,0.0));  //存储各个声道的频谱
    switch (buffer.format().sampleType())
    {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            getBuffers(res, buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            getBuffers(res, buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            getBuffers(res, buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        break;
    case QAudioFormat::Float:
        // 处理浮点型样本
        getBuffers(res, buffer.constData<float>(), buffer.frameCount(), channelCount);
        break;
    case QAudioFormat::SignedInt:
        // 处理有符号整型样本，支持32位、16位和8位
        if (buffer.format().sampleSize() == 32)
            getBuffers(res, buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            getBuffers(res, buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            getBuffers(res, buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        break;
    }
    for(auto& y : res) Standardardize(y,peakValue); //标准化
    for(int i = 0; i < channelCount; ++i) {
        FFT(&res[i].front(),N,&res[i].front());
        res[i].resize(N/2);
        res[i].pop_front();
        //int t = 0;
        for(qreal& x : res[i]) {
            x /= N/2;
            //qDebug() << "频率:" << 1.0*t*buffer.format().sampleRate()/N << ", val: " << x;
            //t++;
        }
    }

    return res;
}

#endif // AUDIODEAL_H
